/*
 * Simple demo program to generate and print out open flow packets.
 * Uses the trema/openflow library.
 */


#include <time.h>
#include <assert.h>
#include "trema.h"
#include <pthread.h>
#include "openflow.h"
#include <inttypes.h>
#include "openflow_message.h"

#define __STDC_FORMAT_MACROS

void
print_buffer( const buffer *buf) {

  struct ofp_flow_mod *flow_mod;
  flow_mod = ( struct ofp_flow_mod * ) buf->data;
  printf("table: %d\n", flow_mod->table_id);

  char *hex = xmalloc( sizeof( char ) * ( buf->length * 2 + 1 ) );
  uint8_t *datap = buf->data;
  char *hexp = hex;
  for ( unsigned int i = 0; i < buf->length; i++, datap++,hexp +=2) {
    printf("%02x", *datap);
  }
  printf("\n");
  xfree( hex );

}

/*
static buffer *
p_create_header( const uint32_t transaction_id, const uint8_t type, const uint16_t length ) {
  debug( "Creating an OpenFlow header (version = %#x, type = %#x, length = %u, xid = %#x).",
         OFP_VERSION, type, length, transaction_id );

  assert( length >= sizeof( struct ofp_header ) );

  buffer *buffer = alloc_buffer();
  assert( buffer != NULL );

  struct ofp_header *header = append_back_buffer( buffer, length );
  assert( header != NULL );
  memset( header, 0, length );

  header->version = OFP_VERSION;
  header->type = type;
  header->length = length;
  header->xid = htonl( transaction_id );

  return buffer;
}

*/

int
main() {

 // struct ofp_flow_mod *of_flow_mod;

  printf("If a packet doesn't match anything, forward it to the controller\n");
  // we do not create a match rule, so it matches everything

  // create actions
  openflow_actions *actions = create_actions();
  append_action_output( actions, OFPP_CONTROLLER, OFPCML_NO_BUFFER );   // sets the output port to that of the OF controller

  // create instructions
  openflow_instructions *insts = create_instructions();
  append_instructions_apply_actions( insts, actions );

  buffer *flow_mod = create_flow_mod(
    get_transaction_id(),    // transaction id: I think this *may* need to be set by the controller
    get_cookie(),            // cookie...same
    0, 			     // cookie mask
    2,                       // table_id : should be 0 till we get to multitable matching
    OFPFC_ADD,               // command: this says ADD the rule
    0,                       // idle time out: rule is removed from table if it doesnt match for this many seconds
    0,                       // hard time out: rule is valid for this many seconds
    OFP_LOW_PRIORITY,        // priority of the rule in the table: Bigger number gets looked at first
    OFP_NO_BUFFER,           // buffer id
    0,                       // output port
    0,                       // out group
    OFPFF_SEND_FLOW_REM,     // flags
    NULL,                    // match rules
    insts                    // instructions
  );
  print_buffer(flow_mod);
  free_buffer( flow_mod);
  delete_instructions(insts);

  
//  of_flow_mod = ( struct ofp_flow_mod * ) flow_mod->data;
//  printf("here: cookie is%" PRId64 "\n", of_flow_mod->cookie);
//  struct ofp_header *header = ( struct ofp_header * ) flow_mod->data;
//  printf("xid is: %u\n",header->xid); 

  // another example with matches
  oxm_matches *match4 = create_oxm_matches();
  append_oxm_match_eth_type( match4, 0x8847);    // match on MPLS EtherType
  append_oxm_match_mpls_label(match4, 4120955);  // match on MPLS label, all 32 bits

  openflow_actions *actions4 = create_actions();
  append_action_pop_mpls(actions4, 0x0800);      // remove the MPLS label and set the ethertype to ARP (or whatever it was before changing to MPLS)
  append_action_output( actions4, 2, OFPCML_NO_BUFFER ); // send packet on port 2

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
  print_buffer(flow_mod4);
  free_buffer( flow_mod4 );
  delete_oxm_matches( match4 );
  delete_instructions( insts4 );

  // a change the dest eth addr
  oxm_matches *match3 = create_oxm_matches();
  append_oxm_match_eth_type( match3, 0x0800);    // match on IP EtherType
  append_oxm_match_ipv4_src( match3, 0xa0a0303,0xffffff00);  // match IP address 10.10.3.xxx 
  
  uint8_t set_src[ OFP_ETH_ALEN ] = {0x0,0x04,0xea,0x31,0x11};
  uint8_t set_dst[ OFP_ETH_ALEN ] = {0x0,0x04,0xe1,0x31,0x15};

  openflow_actions *actions3 = create_actions();
  append_action_push_mpls( actions3, 0x8847 ); 	// push MPLS label, or rather MPLS header
  append_action_set_field_eth_dst( actions3, set_src );  // need to test this
  append_action_set_field_eth_src( actions3, set_dst );  //and this
  append_action_output( actions3, 2, OFPCML_NO_BUFFER ); // send packet on port 2

  openflow_instructions *insts3 = create_instructions();
  append_instructions_apply_actions( insts3, actions3 );

  buffer *flow_mod3 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,                          // cookie mask should be 0 or else cookie bits not masked is expected to match
    0,
    OFPFC_DELETE,               // other commands are OFPFC_MODIFY,OFPFC_MODIFY_STRICT,OFPFC_DELETE_STRICT 
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
  print_buffer(flow_mod3);
  free_buffer( flow_mod3 );
  delete_oxm_matches( match3 );
  delete_instructions( insts3 );



  // delete-flow example with matches
  oxm_matches *match2 = create_oxm_matches();
  append_oxm_match_eth_type( match2, 0x8847);    // match on MPLS EtherType
  append_oxm_match_mpls_label(match2, 4120955);  // match on MPLS label, all 32 bits

  openflow_actions *actions2 = create_actions();
  append_action_pop_mpls(actions2, 0x0800);      // remove the MPLS label and set the ethertype to ARP (or whatever it was before changing to MPLS)
  append_action_output( actions2, 2, OFPCML_NO_BUFFER ); // send packet on port 2

  openflow_instructions *insts2 = create_instructions();
  append_instructions_apply_actions( insts2, actions2 );

  buffer *flow_mod2 = create_flow_mod(
    get_transaction_id(),
    get_cookie(),
    0,                          // cookie mask should be 0 or else cookie bits not masked is expected to match
    0,                  
    OFPFC_DELETE,               // other commands are OFPFC_MODIFY,OFPFC_MODIFY_STRICT,OFPFC_DELETE_STRICT 
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
  print_buffer(flow_mod2);
  free_buffer( flow_mod2 );
  delete_oxm_matches( match2 );
  delete_instructions( insts2 );
 



  return 0;
}


/*
 * Local variables:
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
