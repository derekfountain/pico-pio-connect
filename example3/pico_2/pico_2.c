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

#define TEST_BUFFER_SIZE 16
uint8_t rx_buffer[TEST_BUFFER_SIZE];

int main()
{
  stdio_init_all();  
  sleep_ms(5000);

  printf("Receiving Pico\n\n");

  gpio_init( LED_PIN ); gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 1 );
  
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

  while(1)
  {
    uint8_t data;
    gpio_put( TEST_OUTPUT_GP, 1 );
    receive_buffer( pio0, linkin_sm, rx_buffer, TEST_BUFFER_SIZE );
    gpio_put( TEST_OUTPUT_GP, 0 );

    uint8_t checksum = 0;
    for( int i=0; i < TEST_BUFFER_SIZE; i++ )
    {
      checksum += rx_buffer[i];
    }
    gpio_put( TEST_OUTPUT_GP, 1 );
    send_byte( pio0, linkout_sm, checksum );
    gpio_put( TEST_OUTPUT_GP, 0 );
    
    for( int i=0; i < TEST_BUFFER_SIZE; i++ )
    {
      printf("Received %d : 0x%02X\n", i, rx_buffer[i] );
    }
    printf("\n\n" );
    printf("Sending back checksum of 0x%02X\n\n\n", checksum );
  }
}
