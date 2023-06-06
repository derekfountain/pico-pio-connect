# pico-pio-connect

## Example 2

This example sends one byte, with a known value, from the first Pico
over to the second Pico, which responds by sending a different
value back. The Picos then pause, then the roles reverse, the
second Pico sending the known value and receiving the response.

The Pico's LED flashes twice when it sends, and once when it receives
the expected byte. So the sequence becomes blink-blink on one Pico,
and blink on the other; then it does blink-blink on the second
Pico and blink on the first; then it changes back. Etc.

This two-way link demonstrates the full connection is established
and the Picos can talk to each other.