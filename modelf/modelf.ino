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

	return rc;
}

void loop()
{
	// wait for a falling edge of the clock
	if (digitalRead(AT_CLOCK))
		return;

	// read some bits
	uint16_t bits = at_read();


	uint16_t p = (bits & 0xFF) | 0x1000;
	Serial.print(p, HEX);
	Serial.print(' ');

	// check the parity
	uint8_t count = 1;
	for(int i = 0 ; i < 8 ; i++)
	{
		if (p & 1)
			count++;
		p >>= 1;
	}

	const uint8_t parity = bits & 0x100 ? 1 : 0;
	if ((count & 1) != parity)
	{
		Serial.print(count & 1 ? '1' : '0');
		Serial.print('!'); // not ok
		Serial.print(parity);
		Serial.print(' ');
	}

	// should always be 1
	Serial.println(bits & 0x200 ? '1' : '0');
}
 
