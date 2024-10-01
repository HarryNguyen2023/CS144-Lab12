/******************************************************************************
 * ctcp.c
 * ------
 * Implementation of cTCP done here. This is the only file you need to change.
 * Look at the following files for references and useful functions:
 *   - ctcp.h: Headers for this file.
 *   - ctcp_iinked_list.h: Linked list functions for managing a linked list.
 *   - ctcp_sys.h: Connection-related structs and functions, cTCP segment
 *                 definition.
 *   - ctcp_utils.h: Checksum computation, getting the current time.
 *
 *****************************************************************************/

#include "ctcp.h"
#include "ctcp_linked_list.h"
#include "ctcp_sys.h"
#include "ctcp_utils.h"

/**
 * Connection state.
 *
 * Stores per-connection information such as the current sequence number,
 * unacknowledged packets, etc.
 *
 * You should add to this to store other fields you might need.
 */
struct ctcp_state {
  struct ctcp_state *next;  /* Next in linked list */
  struct ctcp_state **prev; /* Prev in linked list */

  conn_t *conn;             /* Connection object -- needed in order to figure
                               out destination when sending */
  linked_list_t *segments;  /* Linked list of segments sent to this connection.
                               It may be useful to have multiple linked lists
                               for unacknowledged segments, segments that
                               haven't been sent, etc. Lab 1 uses the
                               stop-and-wait protocol and therefore does not
                               necessarily need a linked list. You may remove
                               this if this is the case for you */

  /* FIXME: Add other needed fields. */

};

/**
 * Linked list of connection states. Go through this in ctcp_timer() to
 * resubmit segments and tear down connections.
 */
static ctcp_state_t *state_list;

/* FIXME: Feel free to add as many helper functions as needed. Don't repeat
          code! Helper functions make the code clearer and cleaner. */

/*
  * Flags for types of segment
  * DATA_SEG: data segment
  * ACK_SEG; Acknowledge segment
  * FIN_WITH_ACK: FIN + ACk from web servers
  * FIN_WITH_NO_ACK: FIN from client to terminate conneciton
*/
typedef enum Segment_type
{
  DATA_SEG,
  ACK_SEG,
  FIN_WITH_ACK,
  FIN_WITH_NO_ACK
}Segment_type;

/*
  * Store the information of the ACK of a connection
*/
struct Segment_ACK
{
  bool time_out;
  int time_out_num;
}__attribute__((packed));
typedef struct Segment_ACK Segment_ACK;


ctcp_state_t *ctcp_init(conn_t *conn, ctcp_config_t *cfg) {
  /* Connection could not be established. */
  if (conn == NULL) {
    return NULL;
  }

  /* Established a connection. Create a new state and update the linked list
     of connection states. */
  ctcp_state_t *state = calloc(sizeof(ctcp_state_t), 1);
  state->next = state_list;
  state->prev = &state_list;
  if (state_list)
    state_list->prev = &state->next;
  state_list = state;

  /* Set fields. */
  state->conn = conn;
  /* FIXME: Do any other initialization here. */

  free(cfg);
  return state;
}

void ctcp_destroy(ctcp_state_t *state) {
  /* Update linked list. */
  if (state->next)
    state->next->prev = state->prev;

  *state->prev = state->next;
  conn_remove(state->conn);

  /* FIXME: Do any other cleanup here. */

  free(state);
  end_client();
}

void ctcp_read(ctcp_state_t *state) {
  /* FIXME */
}

/*
  @brief: function to send ACK segmnt

*/
void ctcp_send_ACK(ctcp_state_t *state)
{
  int byte_sent = 0, byte_left, segment_len; 
  segment_len = sizeof(ctcp_segment_t);
  byte_left = segment_len;

  // Fill the ACK segment
  ctcp_segment_t* ack_segment = calloc(1, sizeof(ctcp_segment_t));
  ack_segment->segno = state->conn->seqno;
  ack_segment->ackno = state->conn->ackno;
  ack_segment->len = segment_len;
  ack_segment->flags |= ACK;
  ack_segment->window = MAX_SEG_DATA_SIZE;

  // Get the checksum number of the segment
  ack_segment->cksum = 0;
  ack_segment->cksum = cksum(&ack_segment, segment_len);

  // Send the ACK to the IP socket
  while(byte_left > 0)
  {
    byte_sent = conn_send(state->conn, (ack_segment + segment_len - byte_left), byte_left);
    byte_left -= byte_sent;
  }
  free(ack_segment);
}

void ctcp_receive(ctcp_state_t *state, ctcp_segment_t *segment, size_t len) {
  // Intiiate some variables
  Segment_type cur_seg_type;

  // Check the type of received data
  if(segment->flags & FIN)
  {
    if(segment->flags & ACK)
      cur_seg_type = FIN_WITH_ACK;
    else
      cur_seg_type = FIN_WITH_NO_ACK;
  }
  else if(segment->flags & ACK)
    cur_seg_type = ACK_SEG;
  else
    cur_seg_type = DATA_SEG;
  
  // State machine
  switch(cur_seg_type)
  {
    case DATA_SEG:
    {
      int data_seg_len = len - sizeof(ctcp_segment_t);
      // Update the ACK number of the connection
      state->conn->ackno = segment->segno + 1;
      // Send ACK segemnt back
      ctcp_send_ACK(state);
      // Output data to STDOUT
      conn_output(state->conn, segment->data, data_seg_len);
    }
    break;

    case ACK_SEG:
    {

    }
    break;

    case FIN_WITH_ACK:
    {

    }
    break;

    case FIN_WITH_NO_ACK:
    {

    }
    break;

    default:
    {
      return;
    }
  }

  free(segment);
}

void ctcp_output(ctcp_state_t *state) {
  /* FIXME */
}

void ctcp_timer() {
  // Verify the existence of state list 
  if(state_list == NULL)
    return;

  // initiate some variables
  ctcp_state_t *cur_state = state_list;
  
}
