# AES42HAT firmware

The AES42HAT uses the LPC865M201 as a local control processor (U1). The firmware
for this MCU is contained in this folder here.

The control processor has a number of responsibilities on the AES42HAT:

- Initialize the board
- Collect the user bit and control/status bit data from the AES Receivers (U10,
  U20, U30, U40) via SPI, and present them to the Host processor via I2C.
- Receive the user bit and control/status bit data for the AES Transmitters (U10,
  U20, U30, U40) from the host via I2C, and set them via SPI.
- Measure the relative clock phases between the AES Transmitters, and each of
  the AES Receivers, to support AES42 Mode 2 synchronization.
- Manage phantom powering and remote control of an AES42 compatible source.

The details are described in the succeeding chapters here.
