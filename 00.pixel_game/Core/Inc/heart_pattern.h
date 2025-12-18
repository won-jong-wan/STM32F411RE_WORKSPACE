#ifndef HEART_PATTERN_H
#define HEART_PATTERN_H

#define DATA_PIN    GPIO_PIN_10
#define DATA_PORT   GPIOA
#define LATCH_PIN   GPIO_PIN_5  
#define LATCH_PORT  GPIOB
#define CLOCK_PIN   GPIO_PIN_10
#define CLOCK_PORT  GPIOB

#define PIXEL_ON 1
#define PIXEL_OFF 0

#include <stdint.h>
#include "main.h"


void shift_out(uint8_t data);
void shift_out_lsb(uint8_t data);
void latch_data(void);
void display_heart(uint8_t* pattern);
void pattern_modify(uint8_t* pattern, uint8_t x, uint8_t y, uint8_t on);

#endif // HEART_PATTERN_H
