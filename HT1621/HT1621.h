/*
 * HT1621.h
 *
 *  Created on: 29 Oct 2022.
 *      Author: Alex
 *
 *  The library supports lcd displays based on HT1621 chip
 *  Tested with:
 *  	* 6-digits display
 *  	* 4-digits 'clock' display with semicolon in the middle
 *
 *  The four digits 'clock' display pinout:
 *  1.	VDD
 *  2.	Gnd
 *  3.	Data	(MOSI)
 *  4.	WR		(CLK)
 *  5.	CS
 *  6.	LED+
 *  7.	LED-
 */

#ifndef _HT1621_H_
#define _HT1621_H_

#include <stdbool.h>
#include "main.h"

#define HT1621_SPI_PORT		hspi2
extern SPI_HandleTypeDef 	HT1621_SPI_PORT;

// Comment out the following line to activate normal display, not clock one
#define HT1621_CLOCK

#ifdef __cplusplus
extern "C" {
#endif

void	HT1621_Init(uint8_t size);
void	HT1621_Clear(void);
void	HT1621_Prepare(void);
void	HT1621_Set_Digit(uint8_t seg, uint8_t digit, bool dot);
void	HT1621_Set_Char(uint8_t seg, uint8_t sym, bool dot);
void	HT1621_Set_Raw(uint8_t seg, uint8_t raw);
void	HT1621_Show(void);
void	HT1621_Digit(int32_t data, uint8_t dec_point, bool zero, uint8_t batt_level);
void 	HT1621_Hex_Digit(uint32_t data, bool zero, uint8_t batt_level);
void	HT1621_String(const char *str, uint8_t batt_level);
void 	HT1621_Raw_Data(uint8_t data[], uint8_t size);
void	HT1621_Clock(uint8_t hours, uint8_t minutes, bool semicolumn);
void	HT1621_Date(uint8_t day, uint8_t month);
void	HT1621_ON(void);
void	HT1621_OFF(void);

#ifdef __cplusplus
}
#endif

#endif
