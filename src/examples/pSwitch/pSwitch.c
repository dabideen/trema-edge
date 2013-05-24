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
#include <stdio.h>
#include <time.h>
#include <unistd.h>


time_t
now() {
  return time( NULL );

}


/********************************************************************************
 * packet_in event handler
 ********************************************************************************/


static void
handle_packet_in( uint64_t datapath_id, packet_in message ) {
  info("got packet to handle");
  if ( message.data == NULL ) {
    error( "data must not be NULL" );
    return;
  }
  if ( !packet_type_ether( message.data ) ) {
    return;
  }

  uint32_t in_port = get_in_port_from_oxm_matches( message.match );
  if ( in_port == 0 ) {
    return;
  }

  packet_info packet_info = get_packet_info( message.data );
  
  info("got a packet to handle from, %d",datapath_id);
  info("SA: %d:%d:%d:%d:%d:%d",packet_info.eth_macsa[0],packet_info.eth_macsa[1],packet_info.eth_macsa[2],packet_info.eth_macsa[3],packet_info.eth_macsa[4],packet_info.eth_macsa[5]);

}


/********************************************************************************
 * Start pSwitch controller.
 ********************************************************************************/

static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );
  uint64_t dpid;
  info("switch is ready: %#" PRIx64,datapath_id);
  uint8_t eth_src[6];
  eth_src[5] = datapath_id % 256;
  dpid = datapath_id/256;
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


  uint32_t dest = 3;
//  sleep(2000);

  info("sending ARP config to %d",datapath_id);
  oxm_matches *match2 = create_oxm_matches();
  append_oxm_match_in_port( match2,1);
  append_oxm_match_eth_type( match2, 0x0806);
  append_oxm_match_arp_tpa(match2, 0xa0aff01, 0xa0affff);
  //append_oxm_match_ipv4_src( match2, 0xa0a2803,0xffffff00);

  openflow_actions *actions2 = create_actions();
  append_action_output( actions2, dest, OFPCML_NO_BUFFER );
  append_action_set_field_eth_src( actions2, eth_src );

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
    OFP_HIGH_PRIORITY,
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

  info("sending second config: ipv4");
  oxm_matches *match3 = create_oxm_matches();
  append_oxm_match_eth_type( match3, 0x0800);
  append_oxm_match_in_port( match3,1);
  dest = OFPP_LOCAL;

  openflow_actions *actions3 = create_actions();
  //append_action_push_mpls(actions3,0x8847);
  //append_action_set_field_mpls_label(actions3,412095);
  append_action_output( actions3, dest, OFPCML_NO_BUFFER );

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


  info("sending third config");
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x8847);
//  append_oxm_match_mpls_label(match4, 412095);

  openflow_actions *actions4 = create_actions();
//  append_action_pop_mpls(actions4, 0x0800);
  append_action_output( actions4, 2, OFPCML_NO_BUFFER );
//  append_action_set_field_ip_dscp(actions4, 0x05);

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
  send_openflow_message( datapath_id, flow_mod4);
  free_buffer( flow_mod4 );
  delete_oxm_matches( match4 );
  delete_instructions( insts4 );
  info("done sending");


}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  set_packet_in_handler( handle_packet_in, NULL );

  set_switch_ready_handler( handle_switch_ready, NULL );

  start_trema();

  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
