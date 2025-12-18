#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"

#define DHT11_MOD_IN 1
#define DHT11_MOD_OUT 0

volatile uint16_t tmp = 0;
uint16_t us_counter = 0;
enum state_t {OK, TIMEOUT, VAL_ERR, TRAN_ERR};

extern void delay_us(uint32_t us);

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef* htim){ // ARR=10000-1일때 10ms 마다 호출됨
	if(tmp > 100){ // 1000ms = 1s
		tmp = 0;
	}else{
		tmp++;
	}

	if(tmp == 50){
		if(__HAL_TIM_GET_AUTORELOAD(htim) == 10000-1){
			__HAL_TIM_SET_AUTORELOAD(htim, 5000-1);
		}else if(__HAL_TIM_GET_AUTORELOAD(htim) == 5000-1){
			__HAL_TIM_SET_AUTORELOAD(htim, 10000-1);
		}
	}

	if(tmp < 50){
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	}else{
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	}
}

void dht11_mod_set(uint8_t is_in){

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(is_in){
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
	}else{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
	}

	/*Configure GPIO pin : DHT11_Pin */
	GPIO_InitStruct.Pin = DHT11_Pin;
	HAL_GPIO_Init(DHT11_GPIO_Port, &GPIO_InitStruct);
}

void dht11_error(enum state_t state, uint8_t data[6], int line){
	printf("[line %d] error code: %d\n", line, state);

	printf("data: ");
	for(int i = 0; i<6; i++){
		printf("0x%x ", data[i]);
	}
	printf("\n");
}

void dht11_read_pull(uint8_t target, uint8_t limit_time, enum state_t* state){
	us_counter = 0;

	while(HAL_GPIO_ReadPin(DHT11_GPIO_Port, DHT11_Pin) == target){
		delay_us(2);
		us_counter += 2;
		if(us_counter > limit_time){
			*state = TIMEOUT;
			break;
		}
	}
}

void dht11_main(void){
	enum state_t state = OK;

	uint8_t data[6] = {0};

	// step 0 request start signal //
	dht11_mod_set(DHT11_MOD_OUT);

	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	HAL_Delay(100); // 100ms

	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_RESET);
	HAL_Delay(20); // 20ms

	HAL_GPIO_WritePin(DHT11_GPIO_Port, DHT11_Pin, GPIO_PIN_SET);
	delay_us(30); // 30us

	dht11_mod_set(DHT11_MOD_IN);

	// step 1 response start signal //
	if(state != OK){ dht11_error(state, data, __LINE__); return;}

	dht11_read_pull(0, 100, &state);

	// state == OK
	if(state != OK){ dht11_error(state, data, __LINE__); return;}

	dht11_read_pull(1, 100, &state);

	// step 2 data read //
	if(state != OK){ dht11_error(state, data, __LINE__); return;}

	uint8_t check_sum = 0;

	for(int i=0; i<5; i++){
		uint8_t char_tmp = 0x00;
		for (int j = 0; j<8; j++){
			dht11_read_pull(0, 70, &state);

			if(state != OK) { dht11_error(state, data, __LINE__); return;}

			dht11_read_pull(1, 90, &state);

			if(us_counter > 30){
				char_tmp = char_tmp | 0x01<<(7-j);
			}
		}
		data[i] = char_tmp;
		if(i < 4){
			check_sum += char_tmp;
		}
	}

	// check_sum check //
	if(state != OK){dht11_error(state, data, __LINE__); return;}

	if(data[4] != check_sum){
		state = VAL_ERR;
	}

	// trash val input error //
	delay_us(60);
	// HIGH?
	dht11_read_pull(0, 90, &state);

	if(state != OK){dht11_error(state, data, __LINE__); return;}

	printf("[TEMP]: %d.%dC\n", data[2], data[3]);
	printf("[HOMI]: %d.%d%%\n\n", data[0], data[1]);

	HAL_Delay(1000); // stable time;
}
