# pico-pio-connect

## Example 4

This example is much like example 3 in that it repeatedly sends
a block of data from Pico 1 to Pico 2 over and over. The differences:

* significant code tidy up and some comments
* the block is bigger, and matches the use case I have need for in a
  different project. 135KB goes across in about 645ms.
* the ACK isn't an echo of the received byte, it's a special value. This
  should allow full duplex communications, but I haven't tried it yet.

