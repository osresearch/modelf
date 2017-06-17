/*
 * IBM Model F AT interface
 *
 * This should work with any native USB arduino, such as the Teensy 2.0.  Be
 * sure that the arduino IDE has "USB Type" - "Serial + Keyboard" selected.
 *
 * The clock and data lines are open-collector, multi-master, so we activate
 * pullups and watch for it to pull the clock line low.
 *
 * We can also send commands to query version and configure the LEDs.
 *
 * Command protocol description:
 * http://retired.beyondlogic.org/keyboard/keybrd.htm
 *
 * "The transmission of data in the forward direction, ie Keyboard to Host is
 * done with a frame of 11 bits. The first bit is a Start Bit (Logic 0)
 * followed by 8 data bits (LSB First), one Parity Bit (Odd Parity) and a Stop
 * Bit (Logic 1). Each bit should be read on the falling edge of the clock."
 */

#define AT_CLOCK	5
#define AT_DATA		6

extern "C" uint16_t keymap[];

// is the keyboard signaling a key release to us?
uint8_t release;

// current modifiers held
uint16_t modifiers;

void setup()
{
	pinMode(AT_CLOCK, INPUT_PULLUP);
	pinMode(AT_DATA, INPUT_PULLUP);

	Serial.begin(115200);
	Keyboard.begin();
}


uint16_t at_read()
{
	uint16_t rc = 0;

	for(int i = 0 ; i < 10 ; i++)
	{
		// wait for the clock line to go high
		while(digitalRead(AT_CLOCK) == 0)
			;

		// trigger on the falling edge of the clock
		while(digitalRead(AT_CLOCK) == 1)
			;

		uint8_t bit = digitalRead(AT_DATA);

		rc >>= 1;

		if (bit)
			rc |= 1 << 9;
	}

	// wait for the clock line to go to end the stop bit
	while(digitalRead(AT_CLOCK) == 0)
		;

	return rc;
}

void loop()
{
	// wait for a falling edge of the clock
	if (digitalRead(AT_CLOCK))
		return;

	// read some bits
	const uint16_t bits = at_read();
	const uint8_t scancode = bits & 0xFF;

	// check the parity
	uint8_t count = 1;
	uint8_t p = scancode;
	for(int i = 0 ; i < 8 ; i++)
	{
		if (p & 1)
			count++;
		p >>= 1;
	}

	const uint8_t parity = bits & 0x100 ? 1 : 0;
	if ((count & 1) != parity)
	{
		Serial.print(scancode | 0x1000, HEX);
		Serial.print(' ');
		Serial.print(count & 1 ? '1' : '0');
		Serial.print('!'); // not ok
		Serial.print(parity);

		// should always be 1
		Serial.print(' ');
		Serial.println(bits & 0x200 ? '1' : '0');

		// should signal some sort of error
		release = 0;
		modifiers = 0;
		Keyboard.set_key1(0);
		Keyboard.set_key2(0);
		Keyboard.set_key3(0);
		Keyboard.set_key4(0);
		Keyboard.set_key5(0);
		Keyboard.set_key6(0);
		Keyboard.set_modifier(0);
		Keyboard.send_now();
		return;
	}

	// hopefully we have good data now
	if (scancode >= 0xF0)
	{
		// next key will be a release message
		release = 1;
		return;
	}

	const uint16_t keycode = keymap[scancode];
	if (keycode == 0)
	{
		// unknown key?
		Serial.print(scancode | 0x1000);
		Serial.println("???");
		release = 0;
		return;
	}

	// check to see if this is a modifier
	if (keycode & 0x8000)
	{
		// ignore it for now
		//Serial.print(" mod ");
		//Serial.print(keycode, HEX);

		if (release)
			modifiers &= ~keycode;
		else
			modifiers |= keycode;

		// fixup the high bit in the modifiers
		// if no modifiers are held, set it to zero
		if ((modifiers & 0x7FFF) == 0)
			modifiers = 0;
		else
			modifiers |= 0x8000;

		// send all of our currently held modifiers
		Keyboard.set_modifier(modifiers);
		Keyboard.send_now();

		//Serial.print(' ');
		//Serial.println(modifiers, HEX);

		release = 0;
		return;
	}


	if (release)
		Keyboard.release(keycode);
	else
		Keyboard.press(keycode);

	release = 0;
}
 
