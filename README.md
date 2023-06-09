# pico-pio-connect

This is an example of using the Raspberry Pi Pico's PIO facilities to link
2 Picos together. It's not an application as such, but more like example
code which could be used in a project based on multiple Picos.

## Background

Normally, anyone wanting to connect two Picos together so they can exchange
data would use one of the existing hardware solutions the Pico supports.

UARTs would likely be the first choice, but UARTs are slow. Less than 1Mbps.
Reliable and felxible though.

SPI is much faster, but it's a basic protocol based on master and slave
exchanges. No framing, so not ideal for noisy links. It requires lots of
GPIOs. It's not really suited to peer to peer because it's single master.

I2C is another option. This one has framing which means it's more reliable,
but it's much slower than SPI. The Pico's fast mode gets it to 1Mbps.

## Andrew Menadue's Transputer Link

My project is based on some [Transputer work](https://github.com/blackjetrock/picoputer)
done by Andrew Menadue. The project fills some of the gaps in the Pico's
communications protocol options.

* it's fast, 10Mbps. It might go faster if you work with it;
* it's asynchronous, so no clock and just 2 GPIOs;
* framed on start and stop bits;
* peer to peer, full duplex, so flexible.

You can build on the primitives offered here. My more advanced examples offer
flow control via byte by byte acknowledgement, for example. Andrew's code
builds into a quite complex state machine driven framework.

You don't need to understand the Transputer references in order to use
this, but it might help to appreciate that Andrew was working with the PIOs
to make them compatible with transputer related communications hardware.
Hence some inverted signals and other ostensibly odd design decisions.

The examples flow one into the next. If you're considering your options,
start at the first one.

## Example 1

Extremely simple example which sends a known byte from one Pico to the
other. Confirms the hardware is wired correctly and basic comms can be
established in at least one direction.

## Example 2

This example sends a known byte from one Pico to the other, then the
second Pico sends it back again. Ping pong, confirms the two devices can
talk to each other and both the sending and receiving PIO programs are
working.

## Example 3

Moving on to buffers of data, as opposed to individual bytes. It's mostly
just loops around the single byte primitives, but it also deals with 
unexpected start up noise, flow control and introduces the idea of the
recepient acknowledging every byte transmitted.

---

The examples above are probably all that's required to understand the
concept and get it working. The integration with the user's project is
the next part of the exercise.

This project is GPL, as per Andrew's original code. If you need a different
licence, ask me or Andrew. We flexible on the issue, it depends what you're
doing.

Derek Fountain
June 2023