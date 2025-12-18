#include "heart_pattern.h"

void shift_out(uint8_t data) {
    for(int i = 7; i >= 0; i--) {
        if(data & (1 << i)) {
            HAL_GPIO_WritePin(DATA_PORT, DATA_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(DATA_PORT, DATA_PIN, GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(CLOCK_PORT, CLOCK_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(CLOCK_PORT, CLOCK_PIN, GPIO_PIN_RESET);
    }
}

void shift_out_lsb(uint8_t data) {
    for(int i = 0; i < 8; i++) {
        if(data & (1 << i)) {
            HAL_GPIO_WritePin(DATA_PORT, DATA_PIN, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(DATA_PORT, DATA_PIN, GPIO_PIN_RESET);
        }
        HAL_GPIO_WritePin(CLOCK_PORT, CLOCK_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(CLOCK_PORT, CLOCK_PIN, GPIO_PIN_RESET);
    }
}

void latch_data(void) {
    HAL_GPIO_WritePin(LATCH_PORT, LATCH_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LATCH_PORT, LATCH_PIN, GPIO_PIN_RESET);
}

void display_heart(uint8_t* pattern) {
    for(int cycle = 0; cycle < 100; cycle++) {
        for(int row = 0; row < 8; row++) {
            uint8_t row_data = (1 << row);
            shift_out_lsb(~pattern[row]);
            shift_out(row_data);
            latch_data();
        }
    }
}

void pattern_modify(uint8_t* pattern, uint8_t x, uint8_t y, uint8_t on){
	if(on == PIXEL_ON){
		pattern[7-x] = pattern[7-x] | (0b1<<y);
	}else if(on == PIXEL_OFF){
		pattern[7-x] = pattern[7-x] & ~(0b1<<y);
	}
}
