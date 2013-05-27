/*
 * Simple learning switch application.
 *
 * Author: Yasuhito Takamiya <yasuhito@gmail.com>
 *
 * Copyright (C) 2008-2012 NEC Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <time.h>
#include <assert.h>
#include "trema.h"
#include<pthread.h>
#include <unistd.h>

typedef struct{
  uint64_t src_dpid;
  uint8_t src_port;
  uint64_t dst_dpid;
  uint8_t dst_port;
  time_t timeStamp;
} TopologyEntry;

int num_TE = 20;
TopologyEntry TopologyTable[20];
pthread_t tid;

time_t
now() {
  return time( NULL );

}

uint64_t macToDpid(uint8_t *mac){
  uint64_t dpid = 0;
  for(int i = 0; i < 6; i++){
    dpid = dpid*256 + mac[i];
  }
  return dpid;
}

bool HasTopologyEntry(TopologyEntry *TopologyTable, uint64_t dpid, uint8_t port){
  for(int i = 0; i < num_TE; i++){
    if(TopologyTable[i].src_dpid == dpid && TopologyTable[i].src_port == port){
      return true;
    }
  }
  return false;
}

void UpdateTopologyEntry(TopologyEntry *TopologyTable, uint64_t src_dpid, uint8_t src_port,uint64_t dst_dpid, uint8_t dst_port){
  for(int i = 0; i < num_TE; i++){
    if(TopologyTable[i].src_dpid == src_dpid && TopologyTable[i].src_port == src_port){
      TopologyTable[i].dst_dpid = dst_dpid;
      TopologyTable[i].dst_port = dst_port;
      TopologyTable[i].timeStamp = now();
    }
  }
}

void AddTopologyEntry(TopologyEntry *TopologyTable, uint64_t src_dpid, uint8_t src_port,uint64_t dst_dpid, uint8_t dst_port){
  for(int i = 0; i < num_TE; i++){
    if(TopologyTable[i].src_dpid == 0){
        TopologyTable[i].src_dpid = src_dpid;
        TopologyTable[i].src_port = src_port;
        TopologyTable[i].dst_dpid = dst_dpid;
        TopologyTable[i].dst_port = dst_port;
        TopologyTable[i].timeStamp = now();
        return;
    }
  }
}

void printTopologyTable(TopologyEntry *TopologyTable){
  info(" --------------------");
  info(" Begin Topology Table");
  for(int i = 0; i < num_TE; i++){
    if(TopologyTable[i].src_dpid != 0){
      info("%#" PRIx64 ": %d <===> %#" PRIx64 ": %d",TopologyTable[i].src_dpid,TopologyTable[i].src_port,TopologyTable[i].dst_dpid,TopologyTable[i].dst_port);
    }
  }
  info(" End Topology Table");
  info(" ------------------");
}

void *discoveryPing(){
 while(1){
   system("ping -Q 2 -b 10.10.4.255 -c 1");
   system("ping -Q 3 -b 10.10.4.255 -c 1");
   system("ping -Q 4 -b 10.10.4.255 -c 1");
   system("ping -Q 5 -b 10.10.4.255 -c 1");
   printTopologyTable(TopologyTable);
   sleep(10);
 }

 return NULL;
}

/********************************************************************************
 * packet_in event handler
 ********************************************************************************/


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  //info("got packet to handle");
  if ( message.data == NULL ) {
    error( "data must not be NULL" );
    return;
  }
  if ( !packet_type_ether( message.data ) ) {
    return;
  }

  uint32_t dst_port = get_in_port_from_oxm_matches( message.match );
  if ( dst_port == 0 ) {
    return;
  }

  packet_info packet_info = get_packet_info( message.data );

  //info("got a packet to handle from, %#" PRIx64,datapath_id);
  //info("tos is: %d",packet_info.ipv4_tos);
  uint8_t *src_eth;
  src_eth = packet_info.eth_macsa;
  uint64_t src_dpid = macToDpid(src_eth);
  uint64_t dst_dpid = datapath_id;
  uint8_t src_port = packet_info.ipv4_tos;

  //info("src:  %#" PRIx64,macToDpid(src_eth));

  if(HasTopologyEntry(TopologyTable,src_dpid, src_port)){
    UpdateTopologyEntry(TopologyTable,src_dpid, src_port,dst_dpid,(uint8_t) dst_port);
  }
  else{
    AddTopologyEntry(TopologyTable,src_dpid, src_port,dst_dpid,(uint8_t) dst_port);
    printTopologyTable(TopologyTable);
  }
}



/********************************************************************************
 * Start pSwitch controller.
 ********************************************************************************/

static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

  // convert the dpid to mac address
  uint8_t eth_src[6];
  eth_src[5] = datapath_id % 256;
  uint64_t dpid = datapath_id/256;
  eth_src[4] = dpid % 256;
  dpid = dpid/256;
  eth_src[3] = dpid % 256;
  dpid = dpid/256;
  eth_src[2] = dpid % 256;
  dpid = dpid/256;
  eth_src[1] = dpid % 256;
  dpid = dpid/256;
  eth_src[0] = dpid % 256;


  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_CONTROLLER, OFPCML_NO_BUFFER );

  openflow_instructions *insts = create_instructions();
  append_instructions_apply_actions( insts, actions );

//  uint8_t c1_mac[6] = {0x00,0x04,0x23,0xb7,0x1b,0xe0};
//  uint8_t sw_mac[6] = {0x00,0x04,0x23,0xa8,0xda,0x62};
  //uint8_t sw_mac2[6] = {0x00,0x04,0x23,0xa8,0xda,0x63};
  uint8_t sw2_mac[6] = {0x00,0x04,0x23,0xb7,0x1a,0x0a};
  //uint8_t mask[6] = {0xff,0xff,0xff,0xff,0xff,0xff};

  // default rule: send to ctrl
  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_LOW_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    NULL,
    insts
  );
  send_openflow_message( datapath_id, flow_mod );
  free_buffer( flow_mod );
  delete_instructions( insts );

  uint32_t dest = OFPP_ALL;
//  sleep(2000);

  info("sending ARP config to %d",datapath_id);
  oxm_matches *match2 = create_oxm_matches();
  append_oxm_match_eth_type( match2, 0x0806);

  openflow_actions *actions2 = create_actions();
  append_action_output( actions2, dest, OFPCML_NO_BUFFER );

  openflow_instructions *insts2 = create_instructions();
  append_instructions_apply_actions( insts2, actions2 );

  buffer *flow_mod2 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    12,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match2,
    insts2
  );
  send_openflow_message( datapath_id, flow_mod2 );
  free_buffer( flow_mod2 );
  delete_oxm_matches( match2 );
  delete_instructions( insts2 );

//  sleep(2000);

  info("Topology discovery of port 2 connections");
  oxm_matches *match3 = create_oxm_matches();
  append_oxm_match_eth_type( match3, 0x0800);
  append_oxm_match_ip_dscp(match3,2);
//  append_oxm_match_eth_dst( match3,sw_mac,mask); 
//  append_oxm_match_ipv4_dst( match3,0x0a0a0302,0xffffffff);
  append_oxm_match_in_port( match3, 1);

  openflow_actions *actions3 = create_actions();
  append_action_set_field_eth_src(actions3,eth_src);
  append_action_set_field_eth_dst(actions3,sw2_mac);   // remove me later
  append_action_output(actions3,2,OFPCML_NO_BUFFER );  // send out on port 2
  openflow_instructions *insts3 = create_instructions();
  append_instructions_apply_actions( insts3, actions3 );

  buffer *flow_mod3 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_HIGH_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match3,
    insts3
  );
  send_openflow_message( datapath_id, flow_mod3 );
  free_buffer( flow_mod3 );
  delete_oxm_matches( match3 );
  delete_instructions( insts3 );
  info("done sending");

  info("Topology discovery of port 3 connections");
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x0800);
  append_oxm_match_ip_dscp(match4,3);
//  append_oxm_match_eth_dst( match3,sw_mac,mask); 
//  append_oxm_match_ipv4_dst( match3,0x0a0a0302,0xffffffff);
  append_oxm_match_in_port( match4, 1);

  openflow_actions *actions4 = create_actions();
  append_action_set_field_eth_src(actions4,eth_src);
  append_action_set_field_eth_dst(actions4,sw2_mac);   // remove me later
  append_action_output(actions4,3,OFPCML_NO_BUFFER );  // send out on port 2
  openflow_instructions *insts4 = create_instructions();
  append_instructions_apply_actions( insts4, actions4 );

  buffer *flow_mod4 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_HIGH_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match4,
    insts4
  );
  send_openflow_message( datapath_id, flow_mod4 );
  free_buffer( flow_mod4 );
  delete_oxm_matches( match4 );
  delete_instructions( insts4 );
  info("done sending");


  info("Topology discovery of port 4 connections");
  oxm_matches *match6 = create_oxm_matches();
  append_oxm_match_eth_type( match6, 0x0800);
  append_oxm_match_ip_dscp(match6,4);
  append_oxm_match_in_port( match6,1);

  openflow_actions *actions6 = create_actions();
  append_action_set_field_eth_src(actions6,eth_src);
  append_action_set_field_eth_dst(actions6,sw2_mac);   // remove me later
  append_action_output(actions6,4,OFPCML_NO_BUFFER );  // send out on port 2
  openflow_instructions *insts6 = create_instructions();
  append_instructions_apply_actions( insts6, actions6 );

  buffer *flow_mod6 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_HIGH_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match6,
    insts6
  );
  send_openflow_message( datapath_id, flow_mod6 );
  free_buffer( flow_mod6 );
  delete_oxm_matches( match6 );
  delete_instructions( insts6 );
  info("done sending");



  info("sending local config");
  oxm_matches *match5 = create_oxm_matches();
  append_oxm_match_eth_type( match5, 0x0800);
  append_oxm_match_in_port( match5,1);
  dest = OFPP_LOCAL;   // assumes port 1 is the controller, process IP 

  openflow_actions *actions5 = create_actions();
  append_action_output( actions5, dest, OFPCML_NO_BUFFER );

  openflow_instructions *insts5 = create_instructions();
  append_instructions_apply_actions( insts5, actions5 );

  buffer *flow_mod5 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    10,  // priority
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match5,
    insts5
  );
  send_openflow_message( datapath_id, flow_mod5 );
  free_buffer( flow_mod5 );
  delete_oxm_matches( match5 );
  delete_instructions( insts5 );
  info("done sending");

  // Allow the controller to ARP the node, bypassing OF
     // may not be needed
  info("sending ARP config to %d",datapath_id);
  oxm_matches *match7 = create_oxm_matches();
  append_oxm_match_eth_type( match7, 0x0806);
  append_oxm_match_in_port( match7,1);

  openflow_actions *actions7 = create_actions();
  append_action_output( actions7, dest, OFPCML_NO_BUFFER );

  openflow_instructions *insts7 = create_instructions();
  append_instructions_apply_actions( insts7, actions7 );

  buffer *flow_mod7 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    0,
    0,
    OFP_HIGH_PRIORITY,
    OFP_NO_BUFFER,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match7,
    insts7
  );
  send_openflow_message( datapath_id, flow_mod7 );
  free_buffer( flow_mod7 );
  delete_oxm_matches( match7 );
  delete_instructions( insts7 );

}


void InitTopologyTable(TopologyEntry *TopologyTable){
  for(int i =0; i < num_TE;i++){
    TopologyTable[i].src_dpid = 0;
  }
}      

int
main( int argc, char *argv[] ) {

  InitTopologyTable(TopologyTable);  
  init_trema( &argc, &argv );

  set_packet_in_handler( handle_packet_in, NULL );

  set_switch_ready_handler( handle_switch_ready, NULL );
 
  pthread_create(&tid,NULL,&discoveryPing,NULL);

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
