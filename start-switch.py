#!/usr/bin/python
# -*- coding: utf-8 -*-
""" 
Script to set up CPqD openflow 1.3 softswitch. Also server as a wrapper some dpctl commands.
"""

__all__ = ['run_ifconfig','parse','__main__']

__author__ =  ['Stephen Dabideen <dabideen@bbn.com>']

# Copyright (C) 2013 by 
# Raytheon BBN Technologies.
# All rights reserved. 
# BSD license. 


import subprocess
import re
import sys
import os
import socket
import platform

patterns = [
            '(?P<device>^[a-zA-Z0-9]+).*',
            '.*(inet )addr:(?P<inet>[^\s]*).*',
            '.*(HWaddr )(?P<ether>[^\s]*).*',
            ]

def run_ifconfig():
    """ Function to run ifconfig and store the results. """
    
    process = subprocess.Popen(["ifconfig"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    (stdout, stderr) = process.communicate()
    process.wait()
    return stdout

def parse(ifconfig=None,ctrl_mask='10.10.40'):
    """ Function to parse the output of ifconfig and determines which interface are to be
    controlled by OpenFlow. Assumes the OF interfaces are on the 10.10.xxx.xxx ip range and
    the controller is on a different, specified subnet. """
    
    interfaces = {}
    OF_interfaces = {}
    if not ifconfig:
        print "ifconfig failed"
        return
    cur = None
    all_keys = []

    for line in ifconfig.splitlines():
            for pattern in patterns:
                m = re.match(pattern, line)
                if m:
                    groupdict = m.groupdict()
                    if 'device' in groupdict:
                        cur = groupdict['device']
                        if not interfaces.has_key(cur):
                            interfaces[cur] = {}

                    for key in groupdict:
                        if key not in all_keys:
                            all_keys.append(key)
                        interfaces[cur][key] = groupdict[key]

   # clean up
    for key in interfaces.keys():
        if interfaces[key].has_key('inet'):
            print interfaces[key]['inet']
            if re.match(ctrl_mask,interfaces[key]['inet']):
                dpid = interfaces[key]['ether'].replace(':','')
                dpip = interfaces[key]['inet']
                ceth = key
            elif re.match('10.10.',interfaces[key]['inet']):
                OF_interfaces[key] = interfaces[key]
                OF_interfaces[key]['ether'] = OF_interfaces[key]['ether'].replace(':','')
            #    OF_interfaces[key]['device'] = int(OF_interfaces[key]['device'].replace('eth',''))
        
    return OF_interfaces,dpid,dpip,ceth


if __name__ == "__main__":

    # safty checks 
    if len(sys.argv) < 2:
        print "Insufficient number of arguments. \n Usage: start-switch [ip of controller]." 
        sys.exit()


    ctrl_ip = sys.argv[1]
    ifc = run_ifconfig()
    ctrl_ip_parts = ctrl_ip.split('.')
    ctrl_ip_mask = ctrl_ip_parts[0] + '.' + ctrl_ip_parts[1] + '.' + ctrl_ip_parts[2]
    OF_interfaces,dpid,dpip,ceth = parse(ifc,ctrl_ip_mask)
    OF_interface_list = OF_interfaces.keys()
    OF_interface_list.sort()

    OF_iList = ""
    oflog = open("/tmp/of_config.txt","w")
    for x in range (len(OF_interface_list)):
        if not OF_iList:
            OF_iList = OF_interface_list[x]
            oflog.write(dpid + " " +  str(x+1) + " " + str(OF_interfaces[OF_interface_list[x]]['inet']) + "\n" )
        else:
            oflog.write(dpid + " " + str(x+1) + " " + str(OF_interfaces[OF_interface_list[x]]['inet']) + "\n" )
            OF_iList = OF_iList + "," + OF_interface_list[x]
    oflog.close()
    OF_interface_list.insert(0,ceth)
    cmd = 'screen -d -m sudo /usr/bin/ofdatapath --verbose=dp_acts --verbose=pipeline ptcp:6633 -d ' + dpid + ' -i ' + ','.join(OF_interface_list)  # + ' >/users/dabideen/switch.log'
     
#    print cmd
    os.system(cmd)
    hname = socket.gethostname()
    log = open(hname + "of_config.txt","w")
    port = 1
    for key in OF_interfaces:
       log.write(hname + "\t" + key + "\tMAC: " + OF_interfaces[key]['ether'] + "\tDPID: " + dpid + "\tport: " + str(port) + "\n")
       port = port + 1
    log.close()
    cmd = 'mv ' + hname + 'of_config.txt' + ' ~/'
    os.system(cmd)  

    cmd2 = 'screen -d -m /usr/bin/ofprotocol tcp:' + dpip + ':6633 tcp:' + ctrl_ip + ':6633'
#    print cmd2
    os.system(cmd2)
