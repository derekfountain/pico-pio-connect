/*
 * Pico PIO Connect, a utility to connect Raspberry Pi Picos together
 * Copyright (C) 2023 Andrew Menadue
 * Copyright (C) 2023 Derek Fountain
 * 
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "picoputer.pio.h"
#include "../common/link_common.h"

const uint8_t TEST_OUTPUT_GP  = 0;
const uint8_t LED_PIN         = PICO_DEFAULT_LED_PIN;

const uint8_t LINKOUT_PIN     = 15;
const uint8_t LINKIN_PIN      = 14;

#define TEST_BUFFER_SIZE 16384
uint8_t tx_buffer[TEST_BUFFER_SIZE];

int main()
{
  stdio_init_all();  
  sleep_ms(2000);

  gpio_init( LED_PIN ); gpio_set_dir( LED_PIN, GPIO_OUT ); gpio_put( LED_PIN, 0 );
  
  /* Test pin, blips the scope */
  gpio_init(TEST_OUTPUT_GP); gpio_set_dir(TEST_OUTPUT_GP, GPIO_OUT);
  gpio_put(TEST_OUTPUT_GP, 0);

  /* Set up the link to the other Pico */
  gpio_init(LINKOUT_PIN); gpio_set_dir(LINKOUT_PIN,GPIO_OUT); gpio_put(LINKOUT_PIN, 1);
  gpio_set_function(LINKOUT_PIN, GPIO_FUNC_PIO0);

  gpio_init(LINKIN_PIN); gpio_set_dir(LINKIN_PIN,GPIO_IN);
  gpio_set_function(LINKIN_PIN, GPIO_FUNC_PIO0);
    
  /* Outgoing side of the link */
  int linkout_sm  = pio_claim_unused_sm(pio0, true);
  uint offset     = pio_add_program(pio0, &picoputerlinkout_program);
  picoputerlinkout_program_init(pio0, linkout_sm, offset, LINKOUT_PIN);

  /* Incoming side of the link */
  int linkin_sm   = pio_claim_unused_sm(pio0, true);
  offset          = pio_add_program(pio0, &picoputerlinkin_program);
  picoputerlinkin_program_init(pio0, linkin_sm, offset, LINKIN_PIN);

  gpio_put( LED_PIN, 1 );
  sleep_ms(50);
  gpio_put( LED_PIN, 0 );
  sleep_ms(50);
  gpio_put( LED_PIN, 1 );
  sleep_ms(50);
  gpio_put( LED_PIN, 0 );
  
  /* This side needs to wait for the receiver to start up */
  sleep_ms(2000);

  printf("Sending init string\n" );
  const uint8_t init_msg[] = { 0x02, 0x04, 0x08, 0 };
  send_buffer( pio0, linkout_sm, linkin_sm, init_msg, sizeof(init_msg) );
  uint8_t ack;
  printf("Waiting for ACK\n" );
  while( (receive_acked_byte( pio0, linkin_sm, linkout_sm, &ack ) == LINK_BYTE_NONE)
	 &&
	 (ack != 0xDF) );
  printf("Received\n" );

  while( 1 )
  {
    /* Fill TX buffer */
    uint8_t checksum = 0;
    for( int i=0; i < TEST_BUFFER_SIZE; i++ )
    {
      tx_buffer[i] = (uint8_t)(uint8_t)rand();
      checksum += tx_buffer[i];
    }

    for( int i=0; i < 16; i++ )
    {
      printf("Sending %d : 0x%02X\n", i, tx_buffer[i] );
    }
    gpio_put( TEST_OUTPUT_GP, 1 );
    send_buffer( pio0, linkout_sm, linkin_sm, tx_buffer, TEST_BUFFER_SIZE );
    gpio_put( TEST_OUTPUT_GP, 0 );

    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");
    __asm volatile ("nop");

    uint8_t receiver_checksum;
    gpio_put( TEST_OUTPUT_GP, 1 );
    while( receive_acked_byte( pio0, linkin_sm, linkout_sm, &receiver_checksum ) == LINK_BYTE_NONE );
    gpio_put( TEST_OUTPUT_GP, 0 );

    printf("Calculated checksum 0x%02X\n", checksum );
    printf("Received   checksum 0x%02X\n\n\n", receiver_checksum );

    if( checksum != receiver_checksum )
      while(1);
  }

}
