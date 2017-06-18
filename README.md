IBM Model F Keyboard Interface
===
![IBM Model F USB keyboard](https://farm5.staticflickr.com/4221/35330162436_b938f6c4fd_z_d.jpg)

This is a Arduino/Teensy based adapter for the classic IBM Model F AT
keyboard to a modern USB HID device.  It doesn't require any permanent
modifications to the keyboard and fits entirely inside the spacious
housing.

There are some minor annoyances with this keyboard compared to the
more common Model M:

* There are no arrow keys!
* The escape key is on the numeric keypad, rather than by the tilde.
* The function keys are on the left side, making the keyboard quite wide.
* The backspace is quite small.
* The `\\` and `|` key is oddly positioned next to the backspace.
* The `F` and `J` keys do not have any locator dots.

Wiring
---
![Teensy and Model F PCB](https://farm5.staticflickr.com/4252/35239419941_2bf131733b_z_d.jpg)
There are only four wires necessary to connect the keyboard to the Teensy.
Clock, Data, Power and Ground.  The Model F PCB has long two-row right
angle pin header connector and its cable connected via a 3x2 connector
(with a missing pin) to the far side.  The four pins that we need are:

   Reset (NC)    NC        +5V
   Clock/D0      Ground    Data/D1

The Clock connects to Teensy D0, data to Teensy D1.  Power and Ground
use the USB +5V line.


Protocol
---

The clock and data are open collector, so the AVR is configured with
input pullups and watches for the falling edge on the clock to indicate
that data is arriving.

According to [Craig Peacock's site](http://retired.beyondlogic.org/keyboard/keybrd.htm)

<blockquote>
The transmission of data in the forward direction, ie Keyboard to Host is
done with a frame of 11 bits. The first bit is a Start Bit (Logic 0)
followed by 8 data bits (LSB First), one Parity Bit (Odd Parity) and a Stop
Bit (Logic 1). Each bit should be read on the falling edge of the clock.
</blockquote>

The Teensy busywaits for the falling edge of the clock to indicate
the stop bit, then reads the remaining ten bits in the frame.  It checks
the parity and has a lookup table of IBM scancodes to USB keycodes that
it uses to generate the HID messages.

We can also generate commands to the keyboard to query its version,
enable the LEDs, etc.  There appears to be some sort of problem with
the way that we're dealing with the bus, but it mostly works.
Sometimes toggling caps-lock/num-lock at a high rate causes problems.
teensy won't program, so once the case is closed up it is hard to iterate
