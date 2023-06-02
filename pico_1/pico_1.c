#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/vreg.h"
#include "pico/multicore.h"
#include "picoputer.pio.h"

const uint8_t LED_PIN = PICO_DEFAULT_LED_PIN;

#include "../common/link_common.h"

// The PIOs and SMs of the link with the other Pico
int linkout_sm;
PIO linkout_pio;
int linkin_sm;
PIO linkin_pio;

// Use this if breakpoints don't work
#define DEBUG_STOP {volatile int x = 1; while(x) {} }

const int LINKOUT_PIN  = 15;
const int LINKIN_PIN   = 14;

const int CLOCK_PIN    = 13;

void comfn_send_ram_data_done(void)
{
}


static uint8_t message[] = "Hello, world from Pico 1\n";
void send_ram_data(void)
{
  printf("\nSending message...");
  send_byte_string(message, comfn_send_ram_data_done);
}


void lfsm_ack_received(void)
{
  printf("\nFSM setting IDLE after ACK...");
  link_fsm_stimulus(LFSM_SEND_IDLE, 0);
}



BYTE idle_command[2] = { 1, LINK_CMD_IDLE};
BYTE test_int_command[3] = { 2, LINK_CMD_TEST_INT, 171};

void comfn_idle_done(void)
{
  for( int i=0; i<5; i++ )
  {
    sleep_ms( 100 );
    gpio_put( LED_PIN, 1 );
    sleep_ms( 100 );
    gpio_put( LED_PIN, 0 );
  }

  printf("\nIdle command sent\n");
}

void comfn_test_int_done(void)
{
  // Send the next test int
  //printf("\nIdle command sent\n");
  //  sleep_us(100);
  send_byte_string(test_int_command, comfn_test_int_done);
  test_int_command[2]++;
  if( test_int_command[2] == 0 )
    {
      test_int_command[2]++;
    }
}



int main()
{

  // DEBUG_STOP;

  gpio_init( LED_PIN ); gpio_set_dir( LED_PIN, GPIO_OUT );
  
  stdio_init_all();  
  sleep_ms(2000);

  printf("\n\n");
  printf("\n/------------------------------\\");
  printf("\n| Pico 1                       |");
  printf("\n/------------------------------/");
  printf("\n");
  
  printf("\nSetting PIOs...");
  
  // Set the link to the other Pico up
  gpio_init(LINKOUT_PIN);
  gpio_set_dir(LINKOUT_PIN,GPIO_OUT);
  gpio_put(LINKOUT_PIN, 1);
  gpio_set_function(LINKOUT_PIN, GPIO_FUNC_PIO0);
  
  gpio_init(LINKIN_PIN);
  gpio_set_dir(LINKIN_PIN,GPIO_IN);
  gpio_set_function(LINKIN_PIN, GPIO_FUNC_PIO0);

  //--------------------------------------------------------------------------------
  // Set up link out state machine
  int sm;
  PIO pio;

  pio=pio0;
  
  sm = pio_claim_unused_sm(pio, true);

  linkout_sm = sm;
  linkout_pio = pio;
  
  uint offset = pio_add_program(pio, &picoputerlinkout_program);
  picoputerlinkout_program_init(pio, sm, offset, LINKOUT_PIN);

  sm = pio_claim_unused_sm(pio, true);
  
  linkin_sm = sm;
  linkin_pio = pio;
  offset = pio_add_program(pio, &picoputerlinkin_program);
  picoputerlinkin_program_init(pio, sm, offset, LINKIN_PIN);
  
  gpio_put( LED_PIN, 0 );
  sleep_ms(5000);
  gpio_put( LED_PIN, 1 );
    
  while(1)
  {
//  gpio_put( LED_PIN, 0 );
    sleep_ms(1000);
//  gpio_put( LED_PIN, 1 );

    send_byte_string(idle_command, comfn_idle_done);
//    send_byte_string(test_int_command, comfn_test_int_done);
//    test_int_command[2]++;

    link_process();
  }
}
