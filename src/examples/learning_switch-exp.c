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

typedef struct {
  struct key {
    uint8_t mac[ OFP_ETH_ALEN ];
    uint64_t datapath_id;
  } key;
  uint32_t port_no;
  time_t last_update;
} forwarding_entry;


time_t
now() {
  return time( NULL );
}


/********************************************************************************
 * packet_in event handler
 ********************************************************************************/

static const int MAX_AGE = 300;


static bool
aged_out( forwarding_entry *entry ) {
  if ( entry->last_update + MAX_AGE < now() ) {
    return true;
  }
  else {
    return false;
  };
}


static void
age_forwarding_db( void *key, void *forwarding_entry, void *forwarding_db ) {
  if ( aged_out( forwarding_entry ) ) {
    delete_hash_entry( forwarding_db, key );
    xfree( forwarding_entry );
  }
}


static void
update_forwarding_db( void *forwarding_db ) {
  foreach_hash( forwarding_db, age_forwarding_db, forwarding_db );
}

/*
static void
learn( hash_table *forwarding_db, struct key new_key, uint32_t port_no ) {
  forwarding_entry *entry = lookup_hash_entry( forwarding_db, &new_key );

  if ( entry == NULL ) {
    entry = xmalloc( sizeof( forwarding_entry ) );
    memcpy( entry->key.mac, new_key.mac, OFP_ETH_ALEN );
    entry->key.datapath_id = new_key.datapath_id;
    insert_hash_entry( forwarding_db, &entry->key, entry );
  }
  entry->port_no = port_no;
  entry->last_update = now();
}


static void
do_flooding( packet_in packet_in, uint32_t in_port ) {
  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_ALL, OFPCML_NO_BUFFER );

  buffer *packet_out;
  if ( packet_in.buffer_id == OFP_NO_BUFFER ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      frame
    );
    free_buffer( frame );
  }
  else {
    packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      NULL
    );
  }
  send_openflow_message( packet_in.datapath_id, packet_out );
  free_buffer( packet_out );
  delete_actions( actions );
}

static void
send_packet( uint32_t destination_port, packet_in packet_in, uint32_t in_port ) {
  info("got here");
  uint32_t dest = destination_port;
  oxm_matches *match = create_oxm_matches();
  append_oxm_match_eth_type( match, 0x0806);
  if(in_port == 1){
      info( "config: ARP in 1 -> out 2");
      append_oxm_match_in_port( match, 2);
      dest = 1;
  }
  else{
      info( "config: ARP in 2 -> out 1");
      append_oxm_match_in_port( match, 1);
      dest = 2;
  }

  openflow_actions *actions = create_actions();
  append_action_output( actions, dest, OFPCML_NO_BUFFER );

  openflow_instructions *insts = create_instructions();
  append_instructions_apply_actions( insts, actions );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,
    0,
    OFPFC_ADD,
    60,
    0,
    OFP_HIGH_PRIORITY,
    packet_in.buffer_id,
    0,
    0,
    OFPFF_SEND_FLOW_REM,
    match,
    insts
  );
  send_openflow_message( packet_in.datapath_id, flow_mod );
  free_buffer( flow_mod );
  delete_oxm_matches( match );
  delete_instructions( insts );


  if ( packet_in.buffer_id == OFP_NO_BUFFER ) {
    buffer *frame = duplicate_buffer( packet_in.data );
    fill_ether_padding( frame );
    buffer *packet_out = create_packet_out(
      get_transaction_id(),
      packet_in.buffer_id,
      in_port,
      actions,
      frame
    );
    send_openflow_message( packet_in.datapath_id, packet_out );
    free_buffer( packet_out );
    free_buffer( frame );
  }


  delete_actions( actions );
}
*/

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


  info("got a packet to handle from, %d",datapath_id);
/*
  info("sending third config");
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x8847);
//  append_oxm_match_mpls_label(match4, 4120955);

  openflow_actions *actions4 = create_actions();
  append_action_pop_mpls(actions4, 0x0800);
  append_action_output( actions4, OFPP_ALL, OFPCML_NO_BUFFER );


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
*/

}


/********************************************************************************
 * Start learning_switch controller.
 ********************************************************************************/

static const int AGING_INTERVAL = 5;


unsigned int
hash_forwarding_entry( const void *key ) {
  return hash_mac( ( ( const struct key * ) key )->mac );
}


bool
compare_forwarding_entry( const void *x, const void *y ) {
  const forwarding_entry *ex = x;
  const forwarding_entry *ey = y;
  return ( memcmp( ex->key.mac, ey->key.mac, OFP_ETH_ALEN ) == 0 )
           && ( ex->key.datapath_id == ey->key.datapath_id );
}


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

//  sleep(2000);
  
  info("sending ARP config: flood - %d",datapath_id);
  uint32_t dest = OFPP_ALL;
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

//  sleep(2000);

  info("sending second config: match on eth src");
  uint8_t eth_src[ OFP_ETH_ALEN ] = {0x0,0x24,0xe8,0x77,0xa5,0x32};
  uint8_t set_src[ OFP_ETH_ALEN ] = {0x0,0x10,0x18,0x56,0xab,0xc2};
  uint8_t eth_mask[ OFP_ETH_ALEN ] = {0xff,0xff,0xff,0xff,0xff,0xff};
  oxm_matches *match3 = create_oxm_matches();
  append_oxm_match_eth_type( match3, 0x0800);
  append_oxm_match_eth_src( match3,eth_src,eth_mask);
//  append_oxm_match_in_port( match2, 2);
//  dest = 1;

  openflow_actions *actions3 = create_actions();
  append_action_set_field_eth_src( actions3, set_src );
  append_action_push_mpls(actions3,0x8847);
  append_action_set_field_mpls_label(actions3,412095);
  append_action_output( actions3, 3, OFPCML_NO_BUFFER );

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

//  sleep(2000);

  info("sending config:match mpls label -> pop mpls,  set dest eth");
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x8847);
  append_oxm_match_mpls_label(match4, 412095);

  uint8_t set_src2[ OFP_ETH_ALEN ] = {0x0,0x24,0xe8,0x77,0xa5,0x32};

  openflow_actions *actions4 = create_actions();
  append_action_pop_mpls(actions4, 0x0800);
  append_action_output( actions4, 1, OFPCML_NO_BUFFER );
  append_action_set_field_eth_src( actions4, set_src2 );

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
  send_openflow_message( 0x00101856ab98 , flow_mod4);
  free_buffer( flow_mod4 );
  delete_oxm_matches( match4 );
  delete_instructions( insts4 );


  info("sending second config: match on eth src");
  uint8_t set_src3[ OFP_ETH_ALEN ] = {0x0,0x10,0x18,0x56,0xab,0xc2};
  oxm_matches *match5 = create_oxm_matches();
  append_oxm_match_eth_type( match5, 0x0800);
  
//  append_oxm_match_eth_src( match3,eth_src,eth_mask);
//  append_oxm_match_in_port( match2, 2);
//  dest = 1;

  openflow_actions *actions5 = create_actions();
  append_action_set_field_eth_src( actions5, set_src3 );
  append_action_push_mpls(actions5,0x8847);
  append_action_set_field_mpls_label(actions5,442095);
  append_action_output( actions5, 3, OFPCML_NO_BUFFER );

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
    OFP_HIGH_PRIORITY,
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





}


int
main( int argc, char *argv[] ) {
  init_trema( &argc, &argv );

  hash_table *forwarding_db = create_hash( compare_forwarding_entry, hash_forwarding_entry );
  add_periodic_event_callback( AGING_INTERVAL, update_forwarding_db, forwarding_db );
  set_packet_in_handler( handle_packet_in, forwarding_db );

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
