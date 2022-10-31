/*
 * HT1621.c
 *
 *  Created on: 29 oct 2022
 *      Author: Alex
 */

#include <string.h>
#include "HT1621.h"

/*
 * HT1621 commands
 * Each command starts from there bits, 100 then follows 9 bits of the command parameter
 * The last bit of the command parameter ignored (marked as X)
 * Here the command is stored in one byte (last 8 bits, except ignored X)
 * The first bit of the command shift left to the first three bits of the command mode
 */
typedef enum {
	HT_SYSDIS	= 0X00,		// 100 0000-0000-X, Turn off both system oscillator and LCD bias generator
	HT_SYSEN	= 0x01,		// 100 0000-0001-X, Turn on system oscillator
	HT_LCDOFF	= 0x02,		// 100 0000-0010-X, Turn off LCD bias generator
	HT_LCDON	= 0x03,		// 100 0000-0011-X, Turn on LCD bias generator
	HT_TIMDIS	= 0x04,		// 100 0000-0100-X, Disable time base output
	HT_WDTDIS	= 0x05,		// 100 0000-0101-X, Disable WDT time-out flag output
	HT_TIMEN	= 0x06,		// 100 0000-0110-X, Enable time base output
	HT_WDTEN	= 0x07,		// 100 0000-0111-X, Enable WDT time-out flag output
	HT_TONEOFF	= 0x08,		// 100 0000-1000-X, Turn off tone outputs
	HT_TONEON	= 0x09,		// 100 0000-1001-X, Turn on tone outputs
	HT_CLRTIM	= 0x0C,		// 100 0000-11XX-X, Clear the contents of time base generator
	HT_CLRWDT	= 0x0E,		// 100 0000-111X-X, Clear the contents of WDT stage
	HT_XTAL32K	= 0x14,		// 100 0001-01XX-X, System clock source, crystal oscillator
	HT_RC256K	= 0x18,		// 100 0001-10XX-X, System clock source, on-chip RC oscillator
	HT_EXT256K	= 0x1C,		// 100 0001-11XX-X, System clock source, external clock source
	HT_BIAS12	= 0x20,		// 100 0010-abX0-X, LCD 1/2 bias option: ab=00: 2 commons option; ab=01: 3 commons option; ab=10: 4 commons option
	HT_BIAS13	= 0x21,		// 100 0010-abX1-X, LCD 1/3 bias option: ab=00: 2 commons option; ab=01: 3 commons option; ab=10: 4 commons option
	HT_BIAS		= 0x29,		// 100 0010-1001-X, Bias used to initialize the display, ab = 10
	HT_TONE4K	= 0x40,		// 100 010X-XXXX-X, Tone frequency, 4kHz
	HT_TONE2K	= 0x60,		// 100 011X-XXXX-X, Tone frequency, 2kHz
	HT_IRQDIS	= 0x80,		// 100 100X-0XXX-X, Disable IRQ output
	HT_IRQEN	= 0x88,		// 100 100X-1XXX-X, Enable IRQ output
	HT_F1		= 0xA0,		// 100 101X-X000-X, Time base/WDT clock output:1Hz The WDT time-out flag after: 4s
	HT_F2		= 0xA1,		// 100 101X-X001-X, Time base/WDT clock output:2Hz The WDT time-out flag after: 2s
	HT_F4		= 0xA2,		// 100 101X-X010-X, Time base/WDT clock output:4Hz The WDT time-out flag after: 1s
	HT_F8		= 0xA3,		// 100 101X-X011-X, Time base/WDT clock output:8Hz The WDT time-out flag after: 1/2s
	HT_F16		= 0xA4,		// 100 101X-X100-X, Time base/WDT clock output:16Hz The WDT time-out flag after: 1/4s
	HT_F32		= 0xA5,		// 100 101X-X101-X, Time base/WDT clock output:32Hz The WDT time-out flag after: 1/8s
	HT_F64		= 0xA6,		// 100 101X-X110-X, Time base/WDT clock output:64Hz The WDT time-out flag after: 1/16s
	HT_F128		= 0xA7,		// 100 101X-X111-X, Time base/WDT clock output:128Hz The WDT time-out flag after: 1/32s
	HT_TEST		= 0xE0,		// 100 1110-0000-X, Test mode, user don′t use.
	HT_NORMAL	= 0xE3		// 100 1110-0011-X, Normal mode
} tHT1621_CMD;

typedef struct {
	char	sym;
	uint8_t data;
} tHT1621_ASCII;

#ifdef HT1621_CLOCK
	// Hex digits table
	const static uint8_t hex_digits[16] = {
		0b10101111, 0b10100000,	0b11001011, 0b11101001,	0b11100100,	0b01101101,	0b01101111,	0b10101000,	0b11101111,	0b11101101,
		0b11101110,	0b01100111,	0b00001111,	0b11100011,	0b01001111,	0b01001110
	};

	// Symbol table starting from Ascii 40 code
	const static uint8_t char_table[] = {
	    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00010000, 0b01000000, 0b00010000, 0b00000000,	// 40-47
		0b10101111, 0b10100000,	0b11001011, 0b11101001,	0b11100100,	0b01101101,	0b01101111,	0b10101000,	// 48-55 ('0' - '7')
		0b11101111,	0b11101101, 0B00000000, 0B00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0B00000000, 0b11101110,	0b01100111,	0b00001111,	0b11100011,	0b01001111,	0b01001110, 0B00000000,	// 64-71 ('@' - 'G')
		0b11100110, 0B00000000, 0B00000000, 0B00000000, 0b00000111, 0b00000000, 0b00000000, 0b10101111, // 72-79 ('H' - 'O')
		0B11001110, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000,	// 80-87 ('P' - 'W')
		0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000001,	// 88-95 ('X' - '_')
	};

	const static uint8_t minus 	= 0b01000000;
	const static uint8_t dot	= 0x10;
#else
	// Hex digits table
	const static uint8_t hex_digits[16] = {
		0b01111101, 0b01100000,	0b00111110, 0b01111010,	0b01100011,	0b01011011,	0b01011111,	0b01110000,	0b01111111,	0b01111011,
		0b01110111,	0b01001111,	0b00011101,	0b01101110,	0b00011111,	0b00010111
	};

	// Symbol table starting from Ascii 40 code
	const static uint8_t char_table[] = {
	    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b00000010, 0b10000000, 0b00000000,	// 40-47
		0b01111101, 0b01100000,	0b00111110, 0b01111010,	0b01100011,	0b01011011,	0b01011111,	0b01110000,	// 48-55 ('0' - '7')
		0b01111111,	0b01111011, 0B00000000, 0B00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
		0B00000000, 0b01110111,	0b01001111,	0b00011101,	0b01101110,	0b00011111,	0b00010111, 0B00000000,	// 64-71 ('@' - 'G')
		0b01100111, 0B00000000, 0B00000000, 0B00000000, 0b00001101, 0b00000000, 0b00000000, 0b01111101, // 72-79 ('H' - 'O')
		0B00110111, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000,	// 80-87 ('P' - 'W')
		0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00000000, 0B00001000,	// 88-95 ('X' - '_')
	};

	const static uint8_t minus 	= 0b00000010;
	const static uint8_t dot	= 0x80;
#endif

/*
 * Raw data buffer to load complete 32x4 bits into the display
 * To load complete 16 bytes into the display 9+128 bits have to be sent:
 * 3 bits for the 'write' command, 6 bits for the first address and 128 bits for the data
 * The raw buffer size is 16+2 bytes: zero-st byte reserved for the command and part of display address
 * The display data are stored in 1-16 bytes, the 17-th byte is a trail one ant it is empty
 * Before sending the data into the display, whole raw data array is shifted to right by one bit
 * Actually, 16+128 bits (18 bytes) will be sent into the display (including 7 trailing zero bits)
 * The display size can be customized during initialization.
 */
#define HT1621_RAW_DATA_SIZE	(16)
static  uint8_t raw_data[HT1621_RAW_DATA_SIZE+2] = {0};
static  uint8_t display_size = 16;

static void HT1621_Enable(bool on) {
	HAL_GPIO_WritePin(HT1621_CS_GPIO_Port, HT1621_CS_Pin, on?GPIO_PIN_RESET:GPIO_PIN_SET);
}

static void HT1621_Write_Command(tHT1621_CMD cmd) {
	uint8_t cmd_buff[2];						// 16-bits buffer to send a command. We use 12 bits only

	cmd_buff[0] = (cmd >> 3) | 0x80;			// 0b100 | first 5 bits of the command.
	cmd_buff[1] = cmd << 5;						// remaining 3 bits of the command, trailing '-X' bit and 4 extra zero bits
	HT1621_Enable(true);
	HAL_SPI_Transmit(&HT1621_SPI_PORT, cmd_buff, 2, 100);
	HT1621_Enable(false);
}

static void HT1621_Battery(uint8_t level) {
	if (level > 3) level = 3;
	for (uint8_t i = 0; i < level; ++i)
		raw_data[i+4] |= 0x80;
}

void HT1621_Init(uint8_t size) {
	HT1621_Write_Command(HT_BIAS);
	HT1621_Write_Command(HT_RC256K);
	HT1621_Write_Command(HT_SYSDIS);
	HT1621_Write_Command(HT_WDTDIS);
	HT1621_Write_Command(HT_SYSEN);
	HT1621_Write_Command(HT_LCDON);
	if (size > 16) {
		size = 16;
	} else if (size < 2) {
		size = 2;
	}
	display_size = size;
}

void HT1621_Clear(void) {
	HT1621_Prepare();
	HT1621_Show();
}

void HT1621_Prepare(void) {
	for (uint8_t i = 0; i < display_size+2; ++i)
		raw_data[i] = 0;
}

void HT1621_Set_Digit(uint8_t seg, uint8_t digit, bool dot) {
	if (seg >= display_size || digit > 15)
		return;
	raw_data[seg+1] = hex_digits[digit];
	if (dot)
		raw_data[seg+1] |= dot;
}

void HT1621_Set_Char(uint8_t seg, uint8_t sym, bool dot) {
	if (seg >= display_size || sym < 40 || sym > 95)
		return;
	raw_data[seg+1] = char_table[sym-40];
	if (dot)
		raw_data[seg+1] |= dot;
}

void HT1621_Set_Raw(uint8_t seg, uint8_t raw) {
	if (seg >= display_size)
		return;
	raw_data[seg+1] = raw;
}

void HT1621_Show(void) {
	// First shift whole raw_data array one bit right
	uint16_t shift_buff = 0;
	for (uint8_t i = display_size+1; i >= 1; --i) {
		shift_buff >>= 8;
		shift_buff |= raw_data[i-1] << 8;
		raw_data[i] = (shift_buff >> 1) & 0xff;
	}
	// Write the command to the zero byte
	raw_data[0] = 0xA0;							// 0b10100000, the command 101 + 5 zero bits of the display address
	raw_data[1] &= ~0x80;						// MAke sure the sixth address bit is zero
	HT1621_Enable(true);
	HAL_SPI_Transmit(&HT1621_SPI_PORT, raw_data, display_size + 2, 500);
	HT1621_Enable(false);
}

void HT1621_Digit(int32_t data, uint8_t dec_point, bool zero, uint8_t batt_level) {
	HT1621_Prepare();
	bool negative = false;
	if (data < 0) {
		negative = true;
		data *= -1;
	}
	for (uint8_t i = 0; i < display_size; ++i) {
		uint8_t d = data % 10;
		raw_data[i+1] = hex_digits[d];
		data /= 10;
		if (data == 0) {
			if (negative) {
				if (i < display_size-1)
					raw_data[i+2] = minus;
				break;
			} else if (!zero) {
				break;
			}
		}
	}
	if (dec_point > 0 && dec_point < display_size)
		raw_data[dec_point] |= dot;
	HT1621_Battery(batt_level);
	HT1621_Show();
}

void HT1621_String(const char *str, uint8_t batt_level) {
	HT1621_Prepare();
	uint8_t len = strlen(str);
	if (len > display_size)
		len = display_size;
	for (uint8_t i = 0; i < len; ++i) {
		uint8_t indx = len - i - 1;
		if (str[indx] >= 40 && str[indx] <= 95)
			raw_data[i+1] = char_table[(uint8_t)str[indx] - 40];
	}
	HT1621_Battery(batt_level);
	HT1621_Show();
}

void HT1621_Hex_Digit(uint32_t data, bool zero, uint8_t batt_level) {
	HT1621_Prepare();
	for (uint8_t i = 0; i < display_size; ++i) {
		uint8_t d = data & 0x0f;
		raw_data[i+1] = hex_digits[d];
		data >>= 4;
		if (!data & !zero)
			break;
	}
	HT1621_Battery(batt_level);
	HT1621_Show();
}

void HT1621_Raw_Data(uint8_t data[], uint8_t size) {
	HT1621_Prepare();
	for (uint8_t i = 0; i < display_size; ++i) {
		raw_data[i+1] = data[i];
		if (i >= size)
			break;
	}
	HT1621_Show();
}

void HT1621_Clock(uint8_t hours, uint8_t minutes, bool semicolumn) {
	HT1621_Prepare();
	uint8_t d = minutes % 10;
	raw_data[1] = hex_digits[d];
	d = minutes / 10;
	raw_data[2] = hex_digits[d];
	d = hours % 10;
	raw_data[3] = hex_digits[d];
	d = hours / 10;
	raw_data[4] = hex_digits[d];
	if (semicolumn)
		raw_data[1] |= dot;
	HT1621_Show();
}

void HT1621_Date(uint8_t day, uint8_t month) {
	HT1621_Prepare();
	uint8_t d = month % 10;
	raw_data[1] = hex_digits[d];
	d = month / 10;
	raw_data[2] = hex_digits[d];
	d = day % 10;
	raw_data[3] = hex_digits[d];
	d = day / 10;
	raw_data[4] = hex_digits[d];
	raw_data[3] |= dot;
	HT1621_Show();
}

void HT1621_ON(void) {
	HT1621_Write_Command(HT_LCDON);
}

void HT1621_OFF(void) {
	HT1621_Write_Command(HT_LCDOFF);
}
