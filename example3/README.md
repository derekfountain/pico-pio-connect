# pico-pio-connect

## Example 3

This example sends a block of data (16KB) from one Pico to the other.
A crude checksum byte is calculated and returned. The sequence then
repeats.

Signficant developments here. There's some sort of corruption on the
receiving Pico at startup. When it reads from the FIFO it finds about
half a dozen bytes waiting for it. The scope shows these aren't on the
wire, I don't know what they are. So I added an introduction
sequence which the receiver can look for, throwing away all inbound
data until that sequence is seen.

Also, each byte received is acknowledged by echoing it back to the
sender. This is for flow control. Without it the sender will just
pump out bytes as quickly as its core can push them into the FIFO.
With ACK'ed bytes the sender has to send-then-wait. It can't send
the next byte until it has it confirmed the receiver has received
the previous one. This slows the link down but it prevents bytes
from getting lost if the receiver can't pull them in quickly
enough.

The in-band echoing approach means this method isn't suitable for full
duplex communications. It's not possible to distinguish a data value
from an ACK value.

The 16KB blocks transfer in about 75ms which equates to 218KB/sec.