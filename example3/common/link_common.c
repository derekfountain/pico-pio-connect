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

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "../common/link_common.h"

#undef DEBUG
#define DEBUG 0

void __time_critical_func(blip_test_pin)( int pin )
{
  gpio_put( pin, 1 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  gpio_put( pin, 0 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
}


link_received_t receive_byte( PIO pio, int linkin_sm, uint8_t *received_value )
{
  /* Read from PIO input FIFO */
  uint32_t data;
  if( picoputerlinkin_get( pio, linkin_sm, &data ) == false )
  {
    return LINK_BYTE_NONE;
  }

  /* Invert what's been received */
  data = data ^ 0xFFFFFFFF;

  /* It arrives from the PIO at the top end of the word, so shift down */
  data >>= 22;
      
  /* Remove stop bit in LSB */
  data >>= 1;
	  
  /* Mask out data, just to be sure */
  data &= 0xff;

  *received_value = (uint8_t)data;

  return LINK_BYTE_DATA;
}


void receive_buffer( PIO pio, int linkin_sm, uint8_t *data, uint32_t count )
{
  while( count )
  {
    while( receive_byte( pio0, linkin_sm, data ) == LINK_BYTE_NONE );
    data++;
    count--;
  }
}


void send_byte( PIO pio, int linkout_sm, uint8_t data )
{
  pio_sm_put_blocking(pio, linkout_sm, 0x200 | (((uint32_t)data ^ 0xff)<<1));
}


void send_buffer( PIO pio, int linkout_sm, uint8_t *data, uint32_t count )
{
  while( count )
  {
    send_byte( pio, linkout_sm, *data );
    data++;
    count--;
  }
}
