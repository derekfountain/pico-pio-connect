#include <stdio.h>

#include "hardware/pio.h"
#include "hardware/timer.h"
#include "../common/link_common.h"

#undef DEBUG
#define DEBUG 0

void null_memory_data_handler(void);

uint8_t memory_data_ram_data[MD_ROM_SIZE];
int memory_data_cmd_count = 0;
MDFPTR memory_data_handler = null_memory_data_handler;

uint64_t md_start_time = 0;
uint64_t md_elapsed_time = 0;

////////////////////////////////////////////////////////////////////////////////
//
// Link data
//
////////////////////////////////////////////////////////////////////////////////

int link_data_test_int = 0;

////////////////////////////////////////////////////////////////////////////////
//
// gets any data from input link
//
////////////////////////////////////////////////////////////////////////////////

// Example data received:
//
//  A8800001
//
//  1010 1000 1000 0000 0000 0000 0000 0001
//
//  Real data is AE
//
//  invert:
//  1010 1000 1000 0000 0000 0000 0000 0001
//  0101 0111 0111 1111 1111 1111 1111 1110
//
//  take top 11 bits
//
//  0 1010 1110 1
//
//  0 is the inverted second start bit, 1 is the stop bit
//
//  An ACK will have a top bit of 1

int get_data_from_link(void)
{
  int data;
  
  if( data = picoputerlinkin_get(linkin_pio, linkin_sm) )
    {
#if DEBUG  
      printf("\ndata= %08X", data);	  
#endif

      // We have data
      // We need to invert it and shift it
      data = data ^ 0xFFFFFFFF;
      data >>= 22;
      
#if DEBUG	  
      printf("\ndata= %08X", data);	  
#endif

//      if (data == 0x100 )
//	{
	  // ACK
//	  return(LINK_BYTE_ACK);
//	}
//      else
	{
	  // Data packet
	  // Remove second stop bit in LSB
	  data >>= 1;
	  
	  // Mask out data, just in case
	  data &= 0xff;

	  return(data);
	}
    }

  return(LINK_BYTE_NONE);
}


void send_ack_to_link(void)
{
#if DEBUG
  printf("\nACK sent\n");
#endif
  pio_sm_put_blocking(linkout_pio, linkout_sm, (uint32_t)0x2ff);
}

void send_byte_to_link(int data)
{
#if DEBUG
  printf("\nData sent %02X %d\n", data, data);
#endif

  pio_sm_put_blocking(linkout_pio, linkout_sm, 0x200 | ((data ^ 0xff)<<1));

}


////////////////////////////////////////////////////////////////////////////////
//
// Link processing
//
////////////////////////////////////////////////////////////////////////////////

// The link is based on a byte transfer from the sender to the receiver. Each
// byte is ACKed with the ACK signal.
// The byte stream is parsed as a series of commands which allows
// it to be extended in the future.
//
// Tasks:
//   1. Mirror the RAM/ROM data array in the bus Pico here in the SD Pico
//      so that it can be read and written from and to SD card.
//   2. Wrote to the RAM/ROM data in the bus Pico if data is read from the
//      SD card.
//
// Both Picos use the same protocol in each direction so both have this proessing
// code.
//
// Commands
//
// IDLE: Sent just to keep the link busy
//

////////////////////////////////////////////////////////////////////////////////
//
//

int lfsm_state = LFSM_STATE_IDLE;

int link_idle_count = 0;

void link_fsm_stimulus(int what, int stim)
{
  printf("\nLFSM:state:%d what:%d stim:%d", lfsm_state, what, stim);
  switch(lfsm_state)
    {
    case LFSM_STATE_IDLE:
      switch(what)
	{
	case LFSM_ACK:
	  // ACK for nothing we sent
	  lfsm_ack_received();
	  break;
	  
	case LFSM_DATA:
	  // We have data byte, must be command byte
	  // ACK it
	  send_ack_to_link();

	  // Process command
	  switch(stim)
	    {
	    case LINK_CMD_IDLE:
	      // Just count idles
	      link_idle_count++;
	      if( (link_idle_count % 1000) == 0 )
		{
		  printf("\n1k idles");
		}
	      break;
	    }

	  // Back to idle
	  lfsm_state = LFSM_STATE_IDLE;
	  break;

	case LFSM_SEND_IDLE:
	  printf("\nLFSM_SEND_IDLE\n");
	  send_byte_to_link(LINK_CMD_IDLE);
	  lfsm_state = LFSM_STATE_WAIT_ACK;
	  break;
	  
	default:

	  break;
	  
	}
      break;

    case LFSM_STATE_WAIT_ACK:
      switch(stim)
	{
	case LFSM_ACK:
	  // ACK received, we are done with command
	  lfsm_state = LFSM_STATE_IDLE;
	  lfsm_ack_received();
	  break;
	  
	case LFSM_DATA:
	  // We have data byte, must be command byte
	  break;
	}
      break;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Called repeatedly to process the incoming data
//

int sent_idle = 0;

void link_process2(void)
{
  int data = 0;
  
  data = get_data_from_link();
  
  if( data != LINK_BYTE_NONE)
    {
      // is it an ACK?
      if( data == LINK_BYTE_ACK )
	{
	  printf("\nACK received");
	  link_fsm_stimulus(LFSM_ACK, 0);
	  sent_idle = 0;
	}
      else
	{
	  // Data packet
	  printf("\nDATA:%02X ('%c')", data, data);
	  link_fsm_stimulus(LFSM_DATA, data);
	}
    }
  else
    {
      if( lfsm_state == LFSM_STATE_IDLE,1 )
	{
	  if( !sent_idle )
	    {
	      lfsm_idle_fn();
	      sent_idle = 0;
	    }
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Process incoming bytes
//
////////////////////////////////////////////////////////////////////////////////

// Look for command byte to start a command
typedef void (*COMMAND_FN)(BYTE byte);
boolean processing_command = false;

void cmdfn_null(BYTE byte);

COMMAND_FN command_fn = cmdfn_null;

void cmdfn_null(BYTE byte)
{
}

void cmdfn_memblk(BYTE byte)
{
}

void cmdfn_echo(BYTE byte)
{
}

void cmdfn_test_int(BYTE byte)
{
  // Only one data byte
  processing_command = 0;
  command_fn = cmdfn_null;
  
  link_data_test_int = byte;

  send_ack_to_link();
}

void null_memory_data_handler(void)
{
}

void process_incoming_bytes(BYTE byte)
{
  if( processing_command )
    {
      // Within a command
      // Execute command function
      (*command_fn)(byte);
    }
  else
    {
      // See if this is a comand byte
      switch(byte)
	{
	case LINK_CMD_IDLE:
	  // Do nothing
	  break;
	  
	case LINK_CMD_ECHO:
	  // Send data that should be echoed back

	  break;
	  
	case LINK_CMD_ECHO_REPLY:
	  // Send data that should be echoed back
	  break;
	  
	case LINK_CMD_MEMBLK:
	  // We process this command
	  break;

	case LINK_CMD_TEST_INT:
#if DEBUG
	  printf("\nTEST_INT received\n");
#endif

	  // There is data for this command
	  command_fn = cmdfn_test_int;

	  processing_command = 1;

	  send_ack_to_link();
	  break;
	}
    }
}

////////////////////////////////////////////////////////////////////////////////
//
// Send any remaining bytes from the current byte string
// When done, call the completion function
// Byte strings have length in the first two bytes, so only up to 65535
// bytes can be sent in one string


// Next byte to send
BYTE *byte_just_sent;
COMPLETION_FN completion_fn;
boolean sending_string;
boolean waiting_for_ack = false;
int bytes_to_send = 0;

void send_byte_string(BYTE *string, COMPLETION_FN done)
{
  if( sending_string )
    {
      // Error, can't queue strings, drop it
      printf("\nError, queuing byte strings not supported");
    }
  else
    {
#if 0
      printf("\nSending string:");
      BYTE *s = string;
      while(*s != 0)
	{
	  printf("%02X ", *(s++));
	}
      printf("\n");
#endif

#if DEBUG
      int len = (*string) + (*(string+1)<<8);
      
      printf("\nSending string %d bytes long", len);
      BYTE *s = string;
      int si = 0;
      
      while(*s != 0)
	{
	  if( (si %16) == 0 )
	    {
	      printf("\n");
	    }
	  
	  printf("%02X ", *(s++));
	  si++;
	}
      printf("\n");
	
    
#endif
      
      // Set up a new string to send
      bytes_to_send  = *(string++);
      bytes_to_send |= *(string++) << 8;

#if DEBUG
      printf("\nSending %d bytes of data ", bytes_to_send);
#endif
      byte_just_sent = string;
      completion_fn = done;
      send_byte_to_link(*byte_just_sent);
      bytes_to_send--;
      
      sending_string = true;
      waiting_for_ack = true;
    }
}


////////////////////////////////////////////////////////////////////////////////


void link_process(void)
{
  int data;
  BYTE byte;
  
  // Get data from link
  data = get_data_from_link();
  
  if( data != LINK_BYTE_NONE )
    {
#if DEBUG
      printf("\nRx byte:%02X %d", data, data);
#endif
      if( data == LINK_BYTE_ACK )
	{
#if DEBUG
      printf("\nRx ACK");
#endif
      
	  if( sending_string )
	    {
#if DEBUG
      printf("\nSending string");
#endif

	      if( waiting_for_ack )
		{
#if DEBUG
		  printf("\nWaiting for ACK");
#endif

		  // Last byte Ack'd so send next byte if there is one
		  byte_just_sent++;
		  byte = *byte_just_sent;


		  if( bytes_to_send == 0 )
		    {
#if DEBUG
		      printf("\nLast byte ACKed, all done");
#endif

		      // All done
		      sending_string = false;
		      waiting_for_ack = false;

		      // Call completion function
		      (*completion_fn)();
		    }
		  else
		    {
#if DEBUG
      printf("\nAnother byte to send");
#endif

		      // Another byte to send
		      send_byte_to_link(byte);

		      // Another byte sent
		      bytes_to_send--;
		      waiting_for_ack = true;
		    }
		}
	      else
		{
		  // Not waiting for ACK, warn and ignore
		  printf("\nUnexpected ACK when not expecting one");
		}
	    }
	  else
	    {
	      // ACK when we aren't sending a string, warn
	      printf("\nUnexpected ACK when not sending string");
	    }
	}
      else
	{
#if DEBUG
	  printf("\nRx byte delivered:%02X %d", data, data);
#endif

	  // Data byte received, this is sent on
	  process_incoming_bytes(data);
	  
	  // ACK the byte
	  //send_ack_to_link();
	}
    }
}
