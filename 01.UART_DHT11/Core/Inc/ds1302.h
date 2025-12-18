/*
 * ds1302.h
 *
 *  Created on: Dec 17, 2025
 *      Author: kccistc
 */

#ifndef INC_DS1302_H_
#define INC_DS1302_H_

// write command
// read command = write command + 1
#define ADDR_SEC 0x80
#define ADDR_MIN 0x82
#define ADDR_HOUR 0x84
#define ADDR_DATE 0x86
#define ADDR_MONTH 0x88
#define ADDR_DAY_WEEK 0x8A
#define ADDR_YEAR 0x8C
#define ADDR_WRITE_PROTECTED 0x8E

typedef struct ds1302{
	uint8_t secs;
	uint8_t mins;
	uint8_t hours;
	uint8_t date;
	uint8_t month;
	uint8_t day_week; // 1: sun, 2: mon ...
	uint8_t years;
	uint8_t ampm; // 1: pm, 0: am
	uint8_t hour_mod; // 0: 24, 1: 12
}t_ds1302;

void ds1302_main(void);

#endif /* INC_DS1302_H_ */
