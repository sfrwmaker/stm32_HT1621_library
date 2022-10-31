This is a library to manage the HT1621 based LCD 7-segment display on stm32 micro controller.
The stm32 Cube MX project was created on bluepill board, stm32f103c8t6 micro controller, but it can be easily ported to another stm32 micro controller.
The display requires SPI port working on less than 150 kBit/s bitrate.

The library allocated 18 bytes memory of the micro controller to store the display content and sent the entire data to the display at a time.
To send the data the hardware SPI port used having slow speed (less than 150 kBit/s).
According to the datasheet, the write command (101) followed by the 6-bits address and 128 bits of data takes 137 bits, i.e. 17 bytes and 1 bit, does not alligned
to the byte boundary. The library sends extra 7 zero-bits to use the hardware SPI.
In the command mode the packet size is 11 bits, so the library sends 2 bytes with the trailing zero bites.

To use the library, you have to:
 * specify the SPI port used in the HT1621.h file
 * specify the display type by defining (or comment out) the #define HT1621_CLOCK if you are using the 'clock' version of the display
 * configure SPI port with a low speed, 8-bits; MSB first, CPOL = 1, CPHA = 1 (see the .ioc file for example)

Library function description:
void	HT1621_Init(uint8_t size);
  Initializes the display hardware. The paramener describes the number of the digits on the display (1-15).
  The number of the sending data bits to the display depends on this number.
  The library was tested on 6-digit LCD display with the battery level indicator and on 'clock' display having 4 digits and semicolumn sign.

void	HT1621_Clear(void);
  Clears the display content.
  
void	HT1621_Prepare(void);
  Initializes the internal data buffer for the single segment functions.
  
void	HT1621_Set_Digit(uint8_t seg, uint8_t digit, bool dot);
  Puts the digit (HEX or DEC) into seg segment (0-15 or display size). 0 - is a left symbol. If dot is true, adds the decimal dot to the segment.
  On some display the dot symbol can correspond to the special symbols.
  
void	HT1621_Set_Char(uint8_t seg, uint8_t sym, bool dot);
  Puts the ASCII character to the segment. Suppported characters: space, point, column, minus, digits (0-9), letters A-F, H, O, P, underscore.
  
void	HT1621_Set_Raw(uint8_t seg, uint8_t raw);
  Puts the 'raw' symbol to the segment. This function can be used to check the display symbols mapping.
  
void	HT1621_Show(void);
  Sends the internal data buffer to the display.
  All the data that was setup with HT1621_Set_Digit, HT1621_Set_Char or HT1621_Set_Raw functions will be loaded into the display.

void	HT1621_Digit(int32_t data, uint8_t dec_point, bool zero, uint8_t batt_level);
  Shows the decimal number (positive or negative).
    dec_point defines the segment number where the decimal point will be shown.
    batt_level defines the battery level (0-3). O - no battery displayed. Tested on 6-digit display.
  
void 	HT1621_Hex_Digit(uint32_t data, bool zero, uint8_t batt_level);
  Shows the hexadecimal number.
  
void	HT1621_String(const char *str, uint8_t batt_level);
  Shows the ascii string. Non-existing characters will be replaced by space symbol.
  
void 	HT1621_Raw_Data(uint8_t data[], uint8_t size);
  Shows the 'raw' codes on the segments.
  
void	HT1621_Clock(uint8_t hours, uint8_t minutes, bool semicolumn);
  Shows the time on the 'clock' display. semicolumn allows to show the semicolumn sign.
  
void	HT1621_Date(uint8_t day, uint8_t month);
  Shows the day and month on the 'clock' display. The decimal point will be displayed between the numbers.
  
void	HT1621_ON(void);
  Turn the display on (activates the clock generator).
  
void	HT1621_OFF(void);
  Turn the display off.
