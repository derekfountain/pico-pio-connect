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

#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "picoputer.pio.h"

const uint8_t LED_PIN = PICO_DEFAULT_LED_PIN;

const uint8_t TEST_OUTPUT_GP  = 0;
const uint8_t LINKOUT_PIN     = 15;
const uint8_t LINKIN_PIN      = 14;

int linkin_sm;

uint8_t receive_byte( void )
{
  /* Read from PIO input FIFO */
  uint32_t data = picoputerlinkin_get(pio0, linkin_sm);

  /* Invert what's been received */
  data = data ^ 0xFFFFFFFF;

  /* It arrives from the PIO at the top end of the word, so shift down */
  data >>= 22;
      
  /* Remove stop bit in LSB */
  data >>= 1;
	  
  /* Mask out data, just to be sure */
  data &= 0xff;

  return (uint8_t)data;
}

int main()
{
  gpio_init( LED_PIN ); gpio_set_dir( LED_PIN, GPIO_OUT );
  gpio_put( LED_PIN, 0 );
  
  gpio_init(LINKIN_PIN); gpio_set_dir(LINKIN_PIN,GPIO_IN);
  gpio_set_function(LINKIN_PIN, GPIO_FUNC_PIO0);
  
  /* Outgoing side of the link */
  int linkout_sm  = pio_claim_unused_sm(pio0, true);
  uint offset     = pio_add_program(pio0, &picoputerlinkout_program);
  picoputerlinkout_program_init(pio0, linkout_sm, offset, LINKOUT_PIN);

  /* Incoming side of the link */
  linkin_sm       = pio_claim_unused_sm(pio0, true);
  offset          = pio_add_program(pio0, &picoputerlinkin_program);
  picoputerlinkin_program_init(pio0, linkin_sm, offset, LINKIN_PIN);
  
  /*
   * Ping a data byte back and forth. This side sends a 0xFD, then
   * sits waiting for the other side to respond with 0xDF. Repeats.
   *
   * You should see 1 LED blink, then 2 LED blinks, repeatedly.
   */
  uint8_t sending = 0;

  while(1)
  {
    sleep_ms(1000);

    if( sending )
    {
      gpio_put( LED_PIN, 1 );
      sleep_ms(50);
      gpio_put( LED_PIN, 0 );
      sleep_ms(50);
      gpio_put( LED_PIN, 1 );
      sleep_ms(50);
      gpio_put( LED_PIN, 0 );

      const uint32_t data = 0xFD;
      pio_sm_put_blocking(pio0, linkout_sm, 0x200 | ((data ^ 0xff)<<1));

      sending = 0;
    }
    else
    {
      uint8_t data = receive_byte();

      if( data == 0xDF )
      {
	gpio_put( LED_PIN, 1 );
	sleep_ms(50);
	gpio_put( LED_PIN, 0 );
      }

      sending = 1;
    }
 
  }
}
