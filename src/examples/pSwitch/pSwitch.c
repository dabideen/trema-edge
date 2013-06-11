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
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include <time.h>
#include <assert.h>
#include "trema.h"
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
  //uint32_t dest = 1;
  info("deleting second config: ipv4");
  oxm_matches *match2 = create_oxm_matches();
  //append_oxm_match_eth_type( match2, 0x0800);
  //append_oxm_match_in_port( match2, 2);

  openflow_actions *actions2 = create_actions();
  //append_action_push_mpls(actions2,0x8847);
  //append_action_set_field_mpls_label(actions2,412095);
  //append_action_output( actions2, dest, OFPCML_NO_BUFFER );

  openflow_instructions *insts2 = create_instructions();
  append_instructions_apply_actions( insts2, actions2 );

  buffer *flow_mod2 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_DELETE,
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
  info("done sending");


  info("got a packet to handle from, %d",datapath_id);

}


/********************************************************************************
* Start pSwitch controller.
********************************************************************************/

static void
handle_switch_ready( uint64_t datapath_id, void *user_data ) {
  UNUSED( user_data );

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

  uint32_t dest = OFPP_ALL;
// sleep(2000);
/*

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


  info("deleting ARP config to %d",datapath_id);
  oxm_matches *match3 = create_oxm_matches();
  append_oxm_match_eth_type( match3, 0x0806);

*/


// sleep(2000);

  info("sending second config: ipv4");
  oxm_matches *match3 = create_oxm_matches();
//  append_oxm_match_eth_type( match3, 0x0800);
  append_oxm_match_in_port( match3, 2);
  dest = 1;

  openflow_actions *actions3 = create_actions();
//  append_action_push_mpls(actions3,0x8847);
//  append_action_set_field_mpls_label(actions3,412095);
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

// sleep(2000);

  info("sending third config");
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x8847);
  append_oxm_match_mpls_label(match4, 412095);

  openflow_actions *actions4 = create_actions();
  append_action_pop_mpls(actions4, 0x0800);
  append_action_output( actions4, dest, OFPCML_NO_BUFFER );


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
//  delete_oxm_matches( match4 );
  delete_instructions( insts4 );
  info("done sending");


  info("sending stats request");
  buffer *flow_stat = create_flow_multipart_request(
    get_transaction_id(),
    0,    // flag: don't use 1 unless there are multiple parts of this request
    0,    // table id
    dest,    // out port
    0,    // out group
    get_cookie() ,    // cookie
    0,    // cookie match - ignore cookie
    match4   // match rule
    );
    send_openflow_message( datapath_id, flow_stat);
    free_buffer( flow_stat );
    delete_oxm_matches( match4 );
    info("done");
}


static void
handle_multipart_reply_flow(
  struct ofp_flow_stats *data,
  uint16_t body_length
) {
  struct ofp_flow_stats *stats = data;
  uint16_t rest_length = body_length;
  uint16_t match_len = 0;
  uint16_t match_pad_len = 0;
  uint16_t inst_len = 0;
  struct ofp_instruction *inst;
  int i = 0;
  char inst_str[ 4096 ];
  struct ofp_match *tmp_match;
  oxm_matches *tmp_matches;
  char match_str[ MATCH_STRING_LENGTH ];

  while ( rest_length >= sizeof( struct ofp_flow_stats ) ) {
    struct ofp_flow_stats *next;
    next = ( struct ofp_flow_stats * ) ( ( char * ) stats + stats->length );

    i++;
    info( "[multipart_reply_flow:%d]", i );
    info( " length: %#x", stats->length );
    info( " table_id: %#x", stats->table_id );
    info( " duration_sec: %#x", stats->duration_sec );
    info( " duration_nsec: %#x", stats->duration_nsec );
    info( " priority: %#x", stats->priority );
    info( " idle_timeout: %#x", stats->idle_timeout );
    info( " hard_timeout: %#x", stats->hard_timeout );
    info( " flags: %#x", stats->flags );
    info( " cookie: %#" PRIx64, stats->cookie );
    info( " packet_count: %#" PRIx64, stats->packet_count );
    info( " byte_count: %#" PRIx64, stats->byte_count );
    match_len = stats->match.length;
    match_pad_len = ( uint16_t ) ( match_len + PADLEN_TO_64( match_len ) );
    {
      tmp_match = xcalloc( 1, match_pad_len );
      hton_match( tmp_match, &stats->match );
      tmp_matches = parse_ofp_match( tmp_match );
      match_to_string( tmp_matches, match_str, sizeof( match_str ) );
      xfree( tmp_match );
      delete_oxm_matches( tmp_matches );
    }
    info( " match: [%s]", match_str );
    if ( stats->length > ( offsetof( struct ofp_flow_stats, match ) + match_pad_len ) ) {
      inst_len = ( uint16_t ) ( stats->length - ( offsetof( struct ofp_flow_stats, match ) + match_pad_len ) );
      inst = ( struct ofp_instruction * ) ( ( char * ) stats + offsetof( struct ofp_flow_stats, match ) + match_pad_len );
      instructions_to_string( inst, inst_len, inst_str, sizeof( inst_str ) );
      info( " instructions: [%s]", inst_str );
    }

    rest_length = ( uint16_t ) ( rest_length - stats->length );
    stats = next;
  }
}

static void
handle_multipart_reply(
  uint64_t datapath_id,
  uint32_t transaction_id,
  uint16_t type,
  uint16_t flags,
  const buffer *data,
  void *user_data ) {
  UNUSED( user_data );
  buffer *body = NULL;
  void *multipart_data = NULL;
  uint16_t body_length = 0;

  info( "[multipart_reply]" );
  info( " datapath_id: %#" PRIx64, datapath_id );
  info( " transaction_id: %#x", transaction_id );
  info( " type: %#x", type );
  info( " flags: %#x", flags );

  if ( data != NULL ) {
    body = duplicate_buffer( data );
    multipart_data = body->data;
    body_length = ( uint16_t ) body->length;
  }

  if ( body != NULL ) {
    switch( type ) {
      case OFPMP_FLOW:
        handle_multipart_reply_flow( (struct ofp_flow_stats *) multipart_data, body_length );
        break;
      default:
        info("other type of multipart reply message");

    }
  }

  if ( body != NULL ) {
    free_buffer( body );
  }
}




int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  set_packet_in_handler( handle_packet_in, NULL );

  set_switch_ready_handler( handle_switch_ready, NULL );

  set_multipart_reply_handler( handle_multipart_reply, NULL );

  start_trema();

  return 0;
}


/*
* Local variables:
* c-basic-offset: 2
* indent-tabs-mode: nil
* End:
*/
