/*
 * This is a C file so that we can use designated initializers.
 */
#include "keylayouts.h"

uint16_t keymap[0xF0] = {
	// top row
	[0x0e] = '`',
	[0x16] = '1',
	[0x1e] = '2',
	[0x26] = '3',
	[0x25] = '4',
	[0x2e] = '5',
	[0x36] = '6',
	[0x3d] = '7',
	[0x3e] = '8',
	[0x46] = '9',
	[0x45] = '0',
	[0x4e] = '-',
	[0x55] = '=',
	[0x5d] = '\\',
	[0x66] = KEY_BACKSPACE,

	// second row
	[0x0D] = KEY_TAB,
	[0x15] = 'q',
	[0x1d] = 'w',
	[0x24] = 'e',
	[0x2d] = 'r',
	[0x2c] = 't',
	[0x35] = 'y',
	[0x3c] = 'u',
	[0x43] = 'i',
	[0x44] = 'o',
	[0x4d] = 'p',
	[0x54] = '[',
	[0x5b] = ']',
	[0x5a] = KEY_ENTER,

	// third row
	[0x14] = MODIFIERKEY_LEFT_CTRL,
	[0x1c] = 'a',
	[0x1b] = 's',
	[0x23] = 'd',
	[0x2b] = 'f',
	[0x34] = 'g',
	[0x33] = 'h',
	[0x3b] = 'j',
	[0x42] = 'k',
	[0x4b] = 'l',
	[0x4c] = ';',
	[0x52] = '\'',

	// fourth row
	[0x12] = MODIFIERKEY_LEFT_SHIFT,
	[0x1a] = 'z',
	[0x22] = 'x',
	[0x21] = 'c',
	[0x2a] = 'v',
	[0x32] = 'b',
	[0x31] = 'n',
	[0x3a] = 'm',
	[0x41] = ',',
	[0x49] = '.',
	[0x4a] = '/',
	[0x59] = MODIFIERKEY_RIGHT_SHIFT,

	// fifth row
	[0x11] = MODIFIERKEY_LEFT_ALT,
	[0x29] = ' ',
	[0x58] = KEY_CAPS_LOCK,

	// keypad
	[0x76] = KEY_ESC,
	[0x77] = KEY_NUM_LOCK,
	[0x7e] = KEY_SCROLL_LOCK,
	[0x6c] = KEYPAD_7,
	[0x75] = KEYPAD_8,
	[0x7d] = KEYPAD_9,
	[0x6b] = KEYPAD_4,
	[0x73] = KEYPAD_5,
	[0x74] = KEYPAD_6,
	[0x69] = KEYPAD_1,
	[0x72] = KEYPAD_2,
	[0x7a] = KEYPAD_3,
	[0x70] = KEYPAD_0,
	[0x71] = KEYPAD_PERIOD,
	[0x7c] = KEYPAD_ASTERIX,
	[0x7b] = KEYPAD_MINUS,
	[0x79] = KEYPAD_PLUS,

	// function keys
	[0x05] = KEY_F1,
	[0x06] = KEY_F2,
	[0x04] = KEY_F3,
	[0x0c] = KEY_F4,
	[0x03] = KEY_F5,
	[0x0b] = KEY_F6,
	[0x83] = KEY_F7,
	[0x0a] = KEY_F8,
	[0x01] = KEY_F9,
	[0x09] = KEY_F10,
};
