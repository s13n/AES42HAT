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

## General control processor operation

The control processor is mainly used for configuring the transceiver chips, and
for handling the channel status and user bits of the AES3 / AES42 signals. This
is important since AES42 makes full use of those bits, and it greatly helps a
host processor when this side-band information handling is offloaded into a
specialized processor.

### Handling of C and U bits

The transceiver chips U10, U20, U30 and U40 have a double-buffered memory for
the entire C and U bit information for both receiver and transmitter side. The
buffers can be accessed through the SPI control port. The speed of the SPI port
is sufficient for keeping up with the data rate of 4 transceivers operating in
parallel in both directions (using the I2C bus mode instead wouldn't provide
enough speed). The control processor is intended to read the incoming data on
all channels once for every block of 192 samples. Optionally, it should also be
able to write the outgoing data for all 4 channels for every block of 192
samples, too.

The transmitters are all synchronized to the same clock, therefore the data
blocks can be written in common. The trigger (interrupt) for writing new blocks
comes from the BLS signal. For each trigger, 192 * 8 * 2 bits = 3072 bits = 384
bytes must be written if all channels are to be covered. This is because there
are separate blocks of 192 bits for each of 8 subchannels' C and U bits.

The receivers are clocked individually, and each may operate at a different
frequency. Therefore the data transfer is triggered separately for each. With
each trigger, 768 bits = 96 bytes must be read. The trigger comes from the INT
signal of the respective transceiver, which must be programmed to fire once per
received block.

This means that 5 independent trigger sources compete for SPI bandwidth,
requiring arbitration. The worst case occurs at the highest sampling rates of
~200 kHz for both receivers and transmitters, which leads to a total net data
rate of 800 kByte/s, or 6.4 MBit/s. In practice, the clock rate of the SPI port
will therefore need to be 10 .. 30 MBit/s, which is within the capability of
both the control processor and the transceiver chips.

The SPI interface in the control processor supports DMA, which is useful in this
scenario, to relieve the CPU of handling the actual data transfer.

Note that all 5 trigger signals (BLS, INTA, INTB, INTC, INTD) are connected to
support time stamping with FTM0. This permits phase measurements (see below), as
well as frequency measurements. FTM0 has 6 channels, of which 5 are used for
this purpose.

### Transceiver configuration

The control processor configures the transceiver chips according to the desired
operation mode. This is determined by the host processor, which uses the I2C
interface to set the operation mode by writing control data to the control
processor. The control processor then configures the transceivers accordingly
via SPI0.

### Other functions

The control processor monitors the phantom voltage of all 4 channels using its
ADC (inputs ADC_0 .. ADC_3). It can report the values to the host via the I2C
interface. A significant deviation from the nominal values of 10V or 12V
indicates a hardware problem.

Host communication, microphone synchronization and remote control are described
below in their respective chapters.

### Control processor clock setup

Most of the functions of the control processor don't need to be synchronized
with the audio clock. An internal free running oscillator can clock the chip at
60 MHz, and generate most of the required clocks, including the CPU clock.

For additional flexibility, both MCLK and ACLK signals are routed to the control
processor where they can be used for further clocking related tasks. Either one
or the other can be used to supply the internal `external_clk` signal, which can
serve as the reference for a PLL to generate arbitrary frequencies.

## Mode 2 clock synchronization

When AES42 was developed, one of the problems to solve was how to synchronize
the microphone to a common wordclock, given that the audio signal runs in the
"wrong" direction from the microphone to the controller, where the synchronizing
wordclock needs to go the other way. The solution devised was to establish a
distributed PLL using the remote control facility, which allows low-speed
control signals to be sent by the controller to the microphone via a modulation
scheme of the phantom power. A periodic control word generated by the controller
transmits the phase error to the microphone, where it tunes the clock generator
to produce the correct frequency.

The modulation scheme requires the phantom power of nominally 10 V to be raised
to 12 V in pulses that contain the information. On the AES42HAT, this is done by
switching a boost switchmode regulator between 10 and 12 V, and shaping the
transitions with a pulse driver. This avoids the power dissipation losses of a
linear regulator that is commonly used for this function.

### Controlling pulse timing

The pulse timing is controlled by the control processor, taking advantage of its
FTM1 timer module in conjunction with firmware. In mode 2, the pulses needn't be
synchronous with the wordclock, so the only requirement is to produce the
correct pulse widths according to the AES42 specification. This allows the TIM1
to be clocked internally. FTM1 has 4 channels which are used to control the 4
AES42 channels, so communication can proceed on all channels simultaneously.
Each of the timer channels controls one MODx signal.

The basic idea is to program the TIM1 such that its counter produces one TOF
event per bit that is sent as the remote control signal. Each bit is represented
by a signal that is initially low, and changes to high during the bit period.
For a 0 bit the transition to high happens late during the bit period, while for
a 1 bit it happens early.  This can be generated by using the PWM capability of
the TIM1. During the bit period the counter counts up from its initial value in
CNTIN to its final value in MOD, thereby defining the time for an entire bit.
Somewhere in between, an output transition happens as a consequence of a compare
event. This is what is described as "Edge-Aligned PWM" in the reference manual
of the control processor.

It is possible to trigger a DMA channel on the reloading of the counter (see bit
INITTRIGEN), and have it rewrite the compare registers. This can be used to
reduce the interrupt load by precomputing a list of transition points and
placing it in memory. Alternatively, an interrupt can be generated and the
interrupt service routine takes care of setting the compare registers once for
each bit period.

Firmware bears the responsibility to convert the data to be sent into the
appropriate timing values to use by the TIM1 channel for each bit.

### Phase detection

The control processor implements the phase detection part of the distributed
PLL. It consists of comparing the phase of the incoming AES42 signal with the
local wordclock signal. One phase measurement is needed for each time a control
word is sent to the microphone, which is nominally 6 times per second. Therefore
it is acceptable, and even advantageous, to use the block start signal instead
of the wordclock, to take a phase measurement. This allows aligning the block
start in addition to the wordclock.

The action of the PLL is supposed to minimize the phase error by steering the
clock generator in the microphone. To achieve this, the measured phase error is
filtered and converted into a 13-bit data word which is sent to the microphone 6
times per second. The microphone uses the data word to set the frequency of the
clock generator within a narrow band centered around the nominal frequency.
Traditionally this was done by having a D/A converter control a voltage
controlled crystal oscillator. Other arrangements that produce a similar effect
are also possible, however.

The phase detection is done with the help of TIM0 in capture mode. Its channel 3
captures the edges of the BLS signal, which is the block start signal derived
from the local wordclock. This serves as the reference against which the phase
of the incoming signal from the microphones is measured. The 4 AES42 inputs use
channels 0, 1, 4 and 5 of TIM0, so their phases can be measured simultaneously.
The phase difference is the capture value of the microphone channel minus the
capture value of the reference channel.

The counter of TIM0 is free running, and clocked internally from a clock source
that is unrelated to the audio clocks. This effectively dithers the measurement
results and avoids systematic quantization errors. Additionally, since block
starts are much more frequent than control word transmissions, measurements can
be averaged or smoothed to increase resolution even further.

Implementing phase detection in this way depends on each transceiver chip (U10,
U20, U30, U40) generating an interrupt signal when there is a block start of the
incoming audio signal. This also signals that a complete block of channel status
and user bits has been received and should be fetched using the SPI interface,
so the interrupt signal should not merely cause a capture in TIM0, but the
interrupt service routine should also schedule the data transfer using the SPI
interface.

### Fast mode

The AES42 standard supports two different communication speeds for the remote
control signal, standard mode and fast mode, with support for the latter being
optional for both the microphone and the controller. The implementation
described above for the AES42HAT can support either, but all four channels must
operate in the same mode, because of the usage of a common timer for generating
the timing.

In practice, fast mode would therefore be useable only if all connected
microphones support it. It shouldn't be selected otherwise.

## Mode 3 clock synchronization

As mode 2 synchronization has some disadvantages, Schoeps devised an additional
mode 3 as a different way of remote controlling and synchronizing the
microphone. In mode 3, the synchronization doesn't use the remote control
facility, it is the other way around. The remote control commands get sent
using the synchronization signal as a carrier.

Synchronization is achieved by modulating a low level sinewave onto the phantom
power, with the wordclock frequency as the carrier frequency. On the microphone
side, a narrow band circuit can retrieve this carrier despite its low level. The
advantage is that the chance of interference with the audio signal in the
microphone is greatly reduced, as is the potential radiation from the cable.
Furthermore, it is not necessary to support remote control just for
synchronization. If synchronization is all that's needed, it suffices to just
send the carrier.

The controller needs to be able to generate a sinewave to be superimposed on the
phantom power, and the phase of this sinewave should be directly related to the
local wordclock. This is achieved by bandpass filtering the WCLK signal to
remove harmonics and approximate a sine wave.

The WCLK signal is always at the base frequency, even for double or quadruple
sampling rates, so in practice it is at 44.1 or 48 kHz. This means that there
must be a separate divider to generate it. This can be implemented with the help
of the PLL and the CLKOUT divider, but the SPI1 could also be employed in a
creative way to achieve this.

The remote control signal must be synchronous to the carrier, as each bit that
is sent modulates the amplitude of one full sine wave period. This is achieved
by clocking UART2 with the wordclock signal and using the UART transmit signal
as the modulating signal. Only one microphone can be talked to at any one time
in this way, but since synchronization in mode 3 does not depend on remote
control communication, this isn't a serious drawback.

## Console mode

As an extension to AES42, Schoeps has defined a console mode that uses the user
bit of the right subchannel for a console channel that transmits arbitrary data
in UART format for supporting a textual console connection with the microphone
that can be used during normal operation. Together with an extension of the
remote control command set, this allows bidirectional communication with the
microphone for arbitrary purposes, including an extended command set that
supports functionality beyond what AES42 provides.

Standard AES42 assumes that the user bit of both subchannels contains the same
data, and typically a controller receiving the signal would only look at the
left subchannel. Using the right subchannel therefore doesn't usually impair
normal operation, and the full information AES42 sends in the user bit is still
available.

Console mode is not normally necessary with multiple microphones concurrently.
Therefore the AES42HAT supports using console mode with one microphone at a
time. Multiple microphones can be in console mode concurrently, but only one can
send data at a time, as the AES42HAT can only communicate with one of them.

Towards the host processor, i.e. the Raspberry Pi, the console data is forwarded
from/to its UART interface on Hat pins 8&10, using UART0 of the control
processor. The I2C interface is used to select the microphone to communicate
with.

To support console mode, channel status and user bits must be treated separately
for each of the two subchannels. The AES42HAT is capable of doing this. Incoming
console data in the user bit of the right subchannel can be treated in two
different ways:

### Console mode through SPI0

Here, the data for both user bits and channel status bist for both subchannels
is read by the control processor via SPI0 once for each channel status block
(i.e. once every 192 samples). This is usually done anyway, hence no additional
communication is needed.

The data is decoded in software using the CPU, hence this method causes a
significant CPU load. The problem is mainly to find start bits, which can occur
anywhere in the U-bit stream, and extract the following 8 bits as a data byte.
This bit banging is CPU intensive.

### Console mode through UART1

By using GPO3 and GPO4 of each transceiver chip for the receiver wordclock and
the U-bit, respectively, a situation is created where UART1 can be directly used
to receive the signal in synchronous mode. This saves considerable CPU power by
using the UART hardware intelligently.

## Host communication

The host processor controls the operation of the AES42HAT, i.e. its control
processor, via I2C on pins 3&5 of the Hat connector. The control processor is
the target, and the host processor is the controller. This makes the AES42HAT
appear to the Host processor as a peripheral in much the same way as an audio
codec would be. Configuration is done by writing to registers, and status is
monitored via reading of registers, using the I2C protocol in the usual way.

When the AES42HAT requests the attention of the host processor, it pulls the
REQ/ISP signal low until the host processor services the request. The host
processor would typically react by reading a service request status word via
I2C, which allows it to identify the nature of the request, and service it
accordingly.

The UART interface between host and control processor on pins 8&10 of the Hat
connector is used for remote control of the microphone, and for console mode. It
can also be used for updating the control processor firmware.

### I2C communication

The I2C target of the control processor reacts to a range of I2C 7-bit addresses
like detailed in the following table:

| Addr | Function                        |
|------|---------------------------------|
| 0x70 | U10 passthrough                 |
| 0x71 | U20 passthrough                 |
| 0x72 | U30 passthrough                 |
| 0x73 | U40 passthrough                 |
| 0x74 | board control                   |
| 0x75 | service request status          |
| 0x76 | Remote command buffer           |

The passthrough mode allows the host to access the transceiver chips as if they
were connected directly to the I2C bus. This is intended to help driver software
that was written for a direct connection of the SRC4392 to the I2C bus. The
target addresses are chosen to be compatible with the I2C mode of the SRC4392.
The control processor arbitrates between its own accesses to the transceiver
chips, and those by the host processor.

### UART communication

The UART interface is used for microphone remote control and console mode. One
of the microphones is selected via I2C as the peer to communicate with. When no
microphone is selected, the UART interface can be used as a console interface
with the control processor, if desired.

A baud rate of 230400 bits/s is used with no parity and 1 stop bit. This is
faster than the microphone side, so the possibility exists that the host
overruns the buffering of the AES42HAT, which is handled by XON/XOFF signaling
by the control processor to the host. There is no need for that in the other
direction, as the host should always be quick enough to receive data.

The control processor translates data it receives from the host processor into
remote control pulses for the microphone, according to the selected remote
control and synchronization mode. It can buffer an entire line of at least 80
characters before overrunning.

The control processor forwards console data it receives from the microphone to
the host processor.

## Control processor pin functions

The LPC865 has great flexibility in assigning peripheral functions to pins. We
try to make the best use of this in order to allow some different options for
firmware to take advantage of. However, some functions assignments are
predefined by the chip's architecture, and those restrict the overall pin
function assignments. Other pin assignments not described here are completely
flexible through the switch matrix.

The pin assignments are done such that a 2-channel hardware design can easily be
derived as a spinoff design, with the 32-pin package of the LPC865 being
sufficient. This means that pins `PIO0_29` and onwards are used for channels C
and D only.

Here's the rationale behind the pin assignment:

### Reset, boot and firmware update

The controller runs from an internal boot ROM after reset, and depending on the
state of the `PIO0_12` pin it starts the user program in flash, or it waits for
a firmware image to be provided to UART0 for in-system programming of the flash.
Pulling the pin high, or leaving it getting pulled up by a resistor, results in
the user program to be invoked, which is the normal operating case. Pulling it
low upon reset invokes the ISP handler that sets up UART0 on `PIO0_24` and
`PIO0_25` for receiving commands from the host computer, using a protocol
described in the user manual of the LPC865.

Of course, this requires external control of the reset pin `PIO0_5`. This fixes
the function of the four pins mentioned.

### Clocking

Besides the crystal oscillator on `PIO0_8` and `PIO0_9`, the `CLKIN` function is
also fixed to a single pin, `PIO0_1`. So we use the pins for those functions.

We only need the `XTALIN` function, leaving the `XTALOUT` function free, but we
play it safe and use the latter only as a test point.

The `ACLK` signal on `CLKIN` comes from the clock synthesizer chip, which allows
some flexibility regarding what's fed to the `ACLK` signal. It can be the 10 MHz
reference, or the `RCLK` signal that's routed through, or even the PLL lock
status. Note that it is the host processor that has control over the clock
synthesizer chip, so the controller works with whatever is being presented to
it.

### Debug

Two signals are required for SWD debugging, using pins `PIO0_2` and `PIO0_3`. 

### I2C communication

Two pins, `PIO_10` and `PIO_11`, are specially equipped for I2C usage in their
driving capability, so we use them for that purpose.

### UART communication with the host

We already have assigned UART0 on pins `PIO0_24` and `PIO0_25` for firmware
download, so we keep that for host communication.

### Analog input

The four analog signals `PVA` .. `PVD` need to be connected to one of the inputs
of the controller's ADC. It makes most sense to use `ADC_0` .. `ADC_3`, and thus
pins `PIO0_7`, `PIO0_6`, `PIO0_14` and `PIO0_23`.

The ADC measurement is based on the external reference on pins `VDDA` and
`VSSA`, whose accuracy depends on the switchmode regulator, i.e. it is not very
accurate. It is only suitable for determining whether the phantom voltages are
approximately within the desired range. This is adequate for diagnosing faults.

### FTM0 operation

Pin connectivity of FTM0 is somewhat limited, with only 2 or 3 choices for each
function. We try to allow as much flexibility as possible for firmware to use
FTM0 in controlling the clocking and making phase measurements.

Five of the six available timer channels are needed to measure the wordclock
phases of the 4 inputs against the output wordclock. This is needed for the
implementation of mode 2 synchronization of AES42 sources, but can also be used
in other modes for phase measurements. This functionality is based on the block
synchronization signals that occur once every 192 sample periods. It uses the
input capture mode of the timer channels.

There are two different methods that can be employed for this emasurement. The
synchronous mode and the asynchronous mode. The distinction is whether the
counter is clocked with a clock source that is synchronous with the phase
reference, or whether the clock is independent and asynchrous. There is
something to be said in favor of the asynchronous mode, because the
asynchronicity provides a form of dithering that can help avoiding systematic
effects due to quantization on the time axis. But either mode should be usable
for the task.

The synchronous mode requires the counter to be clocked by a source that is
associated with the local wordclock, i.e. it must ultimately be derived from
`MCLK`. There are two alternatives for arranging this:

- The `MCLK` signal can be used as the "external_clk" signal that can be used
  either directly as the main_clk, or as the reference for the Main PLL inside
  the controller. Either way the resulting clock for the FTM0 will be shared
  with the CPU, which makes the CPU run synchronously with the audio clock, too.
  This may or may not be what is desired for the firmware. Furthermore, it makes
  the `ACLK` signal almost unusable, because it can't feed the "external_clk"
  signal.
- The `BCK` signal can be used to clock the FTM0 counter through `PIO0_30`.
  This is the bit clock at 64* the wordclock frequency. It supports phase
  measurements with the resolution of one bit position in the serial data
  stream. Note that this is not supported with the 32-pin package.

The asynchronous mode clocks the FTM0 counter from an internal clock source,
most likely the FRO, which would also clock the CPU. The asynchronicity will add
a randomness to the sampling point with respect to the wordclock, so that
greater accuracy can be achieved by averaging successive measurements. The PLL
action inherent in the AES42 mode 2 does this anyway.

The block synchronization signal for the transmitting side is `BLS`, which is
generated by channel A and used by the other channels to align their transmitter
phase. It is obtained in U10 by dividing down MCLK. It serves as the reference
against which the phase of the 4 input channels can be measured.

The block synchronization signals for the 4 receivers are independent. Each
channels has two alternatives:

- The `INTx` signal, which must be programmed in each transceiver chip to
  produce an impulse for each received block start. (preferred)
- The `GPO3` output of each transceiver, i.e. the `UCKx` signals. This pin must
  be programmed to produce a pulse for each received block ("Receiver Block
  Start Clock"). This precludes using the pin to clock a UART for doing console
  mode with the help of a UART, i.e. console mode support must be implemented in
  firmware only. On the positive side, the `INTx` signal is free for other use.

The controller is wired up to support either of those alternatives, given the
pin assignment restrictions for FTM0. The `BLS` signal uses `PIO0_17` of timer
channel 0. The four transceivers' interrupt outputs `INTx` use timer channels
2..5 on pins `PIO0_19`, `PIO0_20`, `PIO1_5` and `PIO1_6`. Alternatively, the
`UCKx` signals can be used on pins `PIO0_22`, `PIO0_21`, `PIO1_4` and `PIO1_3`.

Channel 1 of FTM0 can be used in conjunction with the generation of the
`WCLK` signal, which is described in a later section.

### FTM1 operation

Pin connectivity of FTM1 only supports two pin alternatives for each function,
so pin assignments must try to make good use of that. To make matters worse,
some alternatives overlap with FTM0. We try to support the role of FTM1 in
generating the remote control modulation signals in AES42 mode 2 as flexibly as
possible.

The 4 modulation signals are controlled by one of the FTM1 channels each, thus
making full use of the FTM1 capabilities.

The remote control pulses used in mode 2 may or may not be synchronized with the
wordclock. AES42 is not specific about that, however the pulse timing is
described with reference to the "basic sampling frequencies". This suggests that
it would be prudent to be able to synchronize the pulses to the wordclock signal
`WCLK`. Clocking the FTM1 counter from `WCLK` achieves this, which happens
through `PIO0_29`. Again, this is not supported with the 32-pin package.

Alternatively, the counter can be clocked from an internal, asynchronous clock
signal, which results in the remote control pulses running asynchronously from
the wordclock.

The counter channels are operated in output compare mode, with each `MODx` pin
being controlled by one of the FTM1 channels. The pins are assigned accordingly,
to `PIO0_15`, `PIO0_16`, `PIO0_31` and `PIO0_1`.

Note that the `UBTx` signals, which come from the transceivers, use the FTM1
alternative pin assignments `PIO0_27`, `PIO0_26`, `PIO1_9` and `PIO1_8`. This
can be used to support other kinds of measurement functions in conjunction with
whatever signals can be routed to the `GPO4` transceiver outputs.

### WCLK generation

The `WCLK` signal must operate at the basic sampling frequency, hence it can't
be the same as the `LR` signal that can also be at double or quadruple this
frequency, depending on the sampling rate chosen for the transmit side. It is
also desirable for the `WCLK` signal to be controllable in phase. Unfortunately,
no hardware is available inside the controller that fulfills all those demands
directly and simply. There are workarounds, however.

A relatively simple way is to use the `CLKOUT` function on `PIO0_29`, which
requires the Main PLL to be employed. It is not easy to control the phase of the
signal with this method, however.

Better flexibility, including control of the phase, is achievable by employing
SPI1. It works like this:

- Operate SPI1 in slave mode, with the clock coming from `BCK` on pin `PIO0_30`.
- Keep the slave selected continuously, so that a steady data stream results.
- Use the MISO1 output as the `WCLK` signal on `PIO0_29`.
- Use a DMA channel to read cyclically from a pattern buffer in memory.

This permits the SPI1 to output a freely definable serial pattern to be used as
the `WCLK` signal. The firmware controls its shape, including its phase, by
setting the content of the circular buffer, with a resolution defined by the bit
clock.

To measure the phase of the signal, it is possible to employ the help of FTM0
and FTM1 in the following way:

- Clock FTM1 from the `WCLK` signal via `PIO0_29`.
- Program a PWM on `PIO0_16` using FTM1 channel 1.
- Measure it using FTM0 channel 1, taking advantage of the shared pin `PIO0_16`.

This can be done once during startup, as the phase would be stable as a result
of setting up the dividers correctly. The timer ressources can be used for their
normal functions thereafter.

## I2C register maps

### Addresses 0x70 .. 0x73 (SRC4392 passthrough)

### Address 0x74 (Board control)

Board control functions are serviced by the control processor, independently of
the SRC4392 chips. The host processor will normally only need to access this,
and leave SRC4392 handling to the control processor.

| Addr | Register description  |
|------| --------------------- |
| 0x00 | Reset, Initialization |
| .... | ....                  |
| 0x7F | Firmware Version      |

### Address 0x75 (Service request status)

This address needs no register address to be sent. The service request status is
a bitmap that has a bit for each potential service request. As long as there are
unmasked bits set to one in this status word, the REQ signal to the host
processor will be active. The mask is located in the board control register set.
Bits in the service request status remain set until the function they're
representing is serviced.

Simple read transfers are used to read the service request word, which contains
bits to indicate particular service requests, like follows:

TBC

### Address 0x76 (Remote command buffer)

Each channel has a buffer of 64 bytes where commands are stored that are to be
sent to the microphone via AES42 remote commands. The host can select if the
buffer content should be sent once, or repeatedly.

Each command occupies 3 bytes, including the gap between successive commands. In
mode 3, the buffer isn't used, as the remote commands are issued via the UART
connection. When mode 2 is active, the controller inserts Direct Command 3 in
the sequence automatically, to maintain synchronization. The commands in the
buffer are issued in between.

Selecting repeated transmission allows microphone settings to be sent
repeatedly, so that commands issued to the microphone in a different way (e.g.
acoustically) are overridden. If that isn't desired, don't select repeated
transmission.
