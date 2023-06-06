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
#include "hardware/gpio.h"
#include "picoputer.pio.h"

const uint8_t LED_PIN         = PICO_DEFAULT_LED_PIN;

const uint8_t TEST_OUTPUT_GP  = 0;
const uint8_t LINKOUT_PIN     = 15;
const uint8_t LINKIN_PIN      = 14;

static void __time_critical_func(blip_test_pin)( void )
{
  gpio_put( TEST_OUTPUT_GP, 1 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  gpio_put( TEST_OUTPUT_GP, 0 );
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
  __asm volatile ("nop");
}

int main()
{
  gpio_init( LED_PIN ); gpio_set_dir( LED_PIN, GPIO_OUT );
  
  /* Test pin, blips the scope */
  gpio_init(TEST_OUTPUT_GP); gpio_set_dir(TEST_OUTPUT_GP, GPIO_OUT);
  gpio_put(TEST_OUTPUT_GP, 0);

  /* Set up the link to the other Pico */
  gpio_init(LINKOUT_PIN); gpio_set_dir(LINKOUT_PIN,GPIO_OUT); gpio_put(LINKOUT_PIN, 1);
  gpio_set_function(LINKOUT_PIN, GPIO_FUNC_PIO0);
  
  /* Outgoing side of the link */
  int linkout_sm  = pio_claim_unused_sm(pio0, true);
  uint offset     = pio_add_program(pio0, &picoputerlinkout_program);
  picoputerlinkout_program_init(pio0, linkout_sm, offset, LINKOUT_PIN);

  /* This just sends 0xDF out of the PIO link every second. */
  while(1)
  {
    sleep_ms(1000);

    gpio_put( LED_PIN, 1 );
    sleep_ms(50);
    gpio_put( LED_PIN, 0 );

    const uint32_t data = 0xDF;
    pio_sm_put_blocking(pio0, linkout_sm, 0x200 | ((data ^ 0xff)<<1));
  }
}
