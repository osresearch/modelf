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

extern volatile uint8_t keyboard_leds;
static uint8_t last_keyboard_leds;

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

	delay(400);

	// reset the keyboard
	at_send(0xFF);
}


static boolean
wait_clock(
	const uint8_t value,
	const uint16_t timeout_ms
)
{
	// check to see if it is already in the correct state
	if(digitalRead(AT_CLOCK) == !!value)
		return true;

	const uint16_t start_ms = millis();

	while(1)
	{
		// spin a few times checking the status
		for(uint8_t i = 0 ; i < 100 ; i++)
			if(digitalRead(AT_CLOCK) == value)
				return true;

		// have we timed out?
		if (millis() - start_ms > timeout_ms)
			return false;
	}
}


uint16_t at_read(const uint16_t timeout_ms)
{
	uint16_t rc = 0;

/*
	// wait for the low (which might be the current state)
	if (!wait_clock(0, timeout_ms))
	{
		Serial.println("read start timeout");
		return 0xFFFF;
	}
*/

	// clock in the remaining ten bits of data
	for(int i = 0 ; i < 10 ; i++)
	{
		// wait for the clock line to go high
		if (!wait_clock(1, timeout_ms))
		{
			Serial.print(i);
			Serial.println(" rising fail");
			return 0xFFFF;
		}

		// trigger on the falling edge of the clock
		if (!wait_clock(0, timeout_ms))
		{
			Serial.print(i);
			Serial.println(" falling fail");
			return 0xFFFF;
		}

		uint8_t bit = digitalRead(AT_DATA);

		rc >>= 1;

		if (bit)
			rc |= 1 << 9;
	}

	// wait for the clock line to go to end the stop bit
	if (!wait_clock(1, timeout_ms))
	{
		Serial.println("read stop timeout");
		return 0xFFFF;
	}

	return rc;
}


boolean at_send(uint8_t byte)
{
	Serial.print("sending ");
	Serial.println(byte, HEX);

	// make sure they are not sending
	if (digitalRead(AT_CLOCK) == 0)
	{
		Serial.println("BUSY");
		return false;
	}

	// bring the clock low to indicate that we want the bus
	pinMode(AT_CLOCK, OUTPUT);
	digitalWrite(AT_CLOCK, 0);
	delayMicroseconds(10);

	// bring the data low for the start bit
	pinMode(AT_DATA, OUTPUT);
	digitalWrite(AT_DATA, 0);

	// recommended idle
	delayMicroseconds(60);
	const uint16_t timeout_ms = 50;
	uint8_t count = 0;
	uint8_t ack = 0;

	// raise the clock and wait for the keyboard to start clocking
	// in the data from us
	digitalWrite(AT_CLOCK, 1);
	pinMode(AT_CLOCK, INPUT_PULLUP);

	// wait for the clock line to go low and back high
	if (!wait_clock(0, timeout_ms))
	{
		Serial.println("start clock fail");
		goto fail;
	}

	// keyboard is alive; start clocking out the data to it
	for(int i = 0 ; i < 8 ; i++)
	{
		uint8_t bit = byte & 1;
		byte >>= 1;

		// wait for the rising edge
		if (!wait_clock(1, timeout_ms))
		{
			Serial.print(i);
			Serial.println(" rising fail");
			goto fail;
		}

		digitalWrite(AT_DATA, bit);
		if (bit)
			count++;

		// now wait for the clock to come low
		if (!wait_clock(0, timeout_ms))
		{
			Serial.print(i);
			Serial.println(" falling fail");
			goto fail;
		}
	}

	// wait for the rising edge
	if (!wait_clock(1, timeout_ms))
	{
		Serial.println("parity rising fail");
		goto fail;
	}

	// send the parity bit
	digitalWrite(AT_DATA, !(count & 1));

	// wait for the falling edge, indicating that they have clocked it
	if (!wait_clock(0, timeout_ms))
	{
		Serial.println("parity falling fail");
		goto fail;
	}

	// wait for the end of the parity bit
	if (!wait_clock(1, timeout_ms))
	{
		Serial.println("parity end fail");
		goto fail;
	}

	// switch data back into input
	pinMode(AT_DATA, INPUT_PULLUP);

	// wait for the stop bit to start and stop
	if (!wait_clock(0, timeout_ms))
	{
		Serial.println("stop falling fail");
		goto fail;
	}
	if (!wait_clock(1, timeout_ms))
	{
		Serial.println("stop rising fail");
		goto fail;
	}

	// wait for the ack bit to start, then
	// read it from the keyboard; 0 means it got our command
	if (!wait_clock(0, timeout_ms))
	{
		Serial.println("ack falling fail");
		goto fail;
	}

	ack = digitalRead(AT_DATA);

	// and wait for the clock to resume its idle state
	if (!wait_clock(1, timeout_ms))
	{
		Serial.println("ack rising fail");
		goto fail;
	}

	// return the !ack bit == 0 means a successful command cycle
	return !ack;

fail:
	pinMode(AT_DATA, INPUT_PULLUP);
	return false;
}


void at_set_leds(uint8_t leds)
{
	for(int i = 0 ; i < 8 ; i++)
	{
		if (!at_send(0xED))
		{
			Serial.println("FAIL SEND");
			return;
		}

		// we have to wait for an ack
		uint16_t rc = at_read(100);

		if (rc == 0xFFFF)
		{
			Serial.println("FAIL ACK");
			return;
		}

		// ACK?
		if ((rc & 0xFF) == 0xFA)
			break;

		// Resend?
		if ((rc & 0xFF) == 0xFE)
		{
			// don't try forever
			if (i == 7)
				return;
			continue;
		}

		// huh? not what we expected
		Serial.print(rc, HEX);
		Serial.println(" WRONG ACK");
		return;
	}

	uint8_t scroll_lock = leds & 0x04 ? 1 : 0;
	uint8_t caps_lock = leds & 0x02 ? 1 : 0;
	uint8_t num_lock = leds & 0x01 ? 1 : 0;

	// send the LED status byte
	if (!at_send( 0
		| scroll_lock << 0
		| num_lock << 1
		| caps_lock << 2
	))
	{
		Serial.println("FAIL LEDS");
		return;
	}
}


void loop()
{
	// check for a change in the LED status
	static uint8_t scroll_lock;
	if (keyboard_leds != last_keyboard_leds)
	{
		scroll_lock ^= 0x04;
		at_set_leds(keyboard_leds | scroll_lock);
		last_keyboard_leds = keyboard_leds;
	}

	// wait for a falling edge of the clock
	if (digitalRead(AT_CLOCK))
		return;

	// read some bits
	const uint16_t bits = at_read(100);
	if (bits == 0xFFFF)
	{
		//Serial.println("read fail");
		return;
	}

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
		last_keyboard_leds = 0;

		Keyboard.set_key1(0);
		Keyboard.set_key2(0);
		Keyboard.set_key3(0);
		Keyboard.set_key4(0);
		Keyboard.set_key5(0);
		Keyboard.set_key6(0);
		Keyboard.set_modifier(0);
		Keyboard.send_now();

		// reset the keyboard controller if we can
		at_send(0xFF);
		return;
	}

	// hopefully we have good data now
	if (scancode == 0xF0)
	{
		// next key will be a release message
		release = 1;
		return;
	}
	if (scancode >= 0xA0)
	{
		Serial.print(scancode, HEX);
		Serial.println(" message");
		return;
	}

	const uint16_t keycode = keymap[scancode];
	if (keycode == 0)
	{
		// unknown key?
		Serial.print("unknown 0x");
		Serial.print(scancode, HEX);
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
 
