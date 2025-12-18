/*
 * ds1302.c
 *
 *  Created on: Dec 17, 2025
 *      Author: kccistc
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "main.h"
#include "ds1302.h"

//#define DA1302_RESET

#define CORR 2
#define WRITE_MOD 0
#define READ_MOD 1

t_ds1302 ds_time;

extern void delay_us(uint32_t us);

void ds1302_mod_set(uint8_t is_in){

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	if(is_in){
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	}else{
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	}

	GPIO_InitStruct.Pin = IO_DS1302_Pin;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = CE_DS1302_Pin|CLK_DS1302_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void ds1302_data_read(uint8_t* data){ // 30us //most located after ds1302_com_write()
	*data = 0x00;

	ds1302_mod_set(READ_MOD);

	for(int i=0; i< 8; i++){
		// data read
		(*data) = (*data) | HAL_GPIO_ReadPin(IO_DS1302_GPIO_Port, IO_DS1302_Pin) << i;

		if(i == 7) break;

		// rising
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_SET);
		delay_us(15-CORR); // 2us 정도의 밀림 고려

		// falling
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_RESET);
		delay_us(15-CORR);
	}

	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, GPIO_PIN_RESET);
}

void ds1302_data_write(uint8_t data){ // 30us //most located after ds1302_com_write()
	for(int i=0; i< 8; i++){
		// data write
		HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, data & (0x01<< i));
		// rising
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_SET);
		delay_us(15-CORR); // 2us 정도의 밀림 고려

		// falling
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_RESET);
		delay_us(15-CORR);
	}

	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, GPIO_PIN_RESET);
}

void ds1302_com_write(uint8_t command){ // 30us
	ds1302_mod_set(WRITE_MOD);

	// Enable
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, GPIO_PIN_SET);

	for(int i=0; i< 8; i++){
		// data write
		HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, command & (0x01<< i));
		// rising
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_SET);
		delay_us(15-CORR); // 2us 정도의 밀림 고려

		// falling
		HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_RESET);
		delay_us(15-CORR);
	}
}

void ds1302_write(t_ds1302 ds_time){
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, GPIO_PIN_RESET);

	delay_us(10);

	ds_time.secs = ((ds_time.secs/10) << 4) | (ds_time.secs%10);
	ds1302_com_write(ADDR_SEC);
	ds1302_data_write(ds_time.secs);

	ds_time.mins = ((ds_time.mins/10) << 4) | (ds_time.mins%10);
	ds1302_com_write(ADDR_MIN);
	ds1302_data_write(ds_time.mins);

	ds_time.hours = ((ds_time.hours/10) << 4) | (ds_time.hours%10);
	ds_time.hours = ds_time.hours | ds_time.hour_mod << 7;
	ds1302_com_write(ADDR_HOUR);
	ds1302_data_write(ds_time.hours);

	ds_time.date = ((ds_time.date/10) << 4) | (ds_time.date%10);
	ds1302_com_write(ADDR_DATE);
	ds1302_data_write(ds_time.date);

	ds_time.month = ((ds_time.month/10) << 4) | (ds_time.month%10);
	ds1302_com_write(ADDR_MONTH);
	ds1302_data_write(ds_time.month);

//	ds1302_com_write(ADDR_DAY_WEEK);
//	ds1302_data_write(ds_time.day_week);

	ds_time.years = ((ds_time.years/10) << 4) | (ds_time.years%10);
	ds1302_com_write(ADDR_YEAR);
	ds1302_data_write(ds_time.years);
}

void ds1302_read(t_ds1302* ds_time){
	HAL_GPIO_WritePin(CE_DS1302_GPIO_Port, CE_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(CLK_DS1302_GPIO_Port, CLK_DS1302_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(IO_DS1302_GPIO_Port, IO_DS1302_Pin, GPIO_PIN_RESET);

	delay_us(10);

	ds1302_com_write(ADDR_SEC+1);
	ds1302_data_read(&(ds_time->secs));
	ds_time->secs = ((ds_time->secs & 0x70) >> 4)*10 + (ds_time->secs & 0x0F);

	ds1302_com_write(ADDR_MIN+1);
	ds1302_data_read(&(ds_time->mins));
	ds_time->mins = ((ds_time->mins & 0x70) >> 4)*10 + (ds_time->mins & 0x0F);

	ds1302_com_write(ADDR_HOUR+1);
	ds1302_data_read(&(ds_time->hours));
	ds_time->hour_mod = ds_time->hours & 0x80;
	ds_time->ampm = ds_time->hours & 0x20;
	ds_time->hours = ((ds_time->hours & 0x30) >> 4)*10 + (ds_time->hours & 0x0F);

	ds1302_com_write(ADDR_DATE+1);
	ds1302_data_read(&(ds_time->date));
	ds_time->date = ((ds_time->date & 0x30) >> 4)*10 + (ds_time->date & 0x0F);

	ds1302_com_write(ADDR_MONTH+1);
	ds1302_data_read(&(ds_time->month));
	ds_time->month = ((ds_time->month & 0x10) >> 4)*10 + (ds_time->month & 0x0F);

	ds1302_com_write(ADDR_DAY_WEEK+1);
	ds1302_data_read(&(ds_time->day_week));
	ds_time->day_week = (ds_time->day_week & 0x07);

	ds1302_com_write(ADDR_YEAR+1);
	ds1302_data_read(&(ds_time->years));
	ds_time->years = ((ds_time->years & 0xF0) >> 4)*10 + (ds_time->years & 0x0F);
}

extern uint8_t recv_val[BUF_SIZE];

void recv_parsing(){
	// setrtc251218094000
	// yymmddhhmmss
	char* cmd = strstr((char *)recv_val, "setrtc");

	char* hours = strstr((char *)recv_val, "h=");
	char* mins = strstr((char *)recv_val, "m=");
	char* secs = strstr((char *)recv_val, "s=");

	if(cmd != 0){
		char* cmd_h = cmd+6;
	//	printf("cmd_h : %s\n", cmd_h);

		ds_time.years = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
		cmd_h += 2;
		ds_time.month = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
		cmd_h += 2;
		ds_time.day_week = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
		cmd_h += 2;
		ds_time.hours = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
		cmd_h += 2;
		ds_time.mins = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
		cmd_h += 2;
		ds_time.secs = (*(cmd_h)-'0')*10 + (*(cmd_h+1)-'0');
	}

	if(hours != 0) ds_time.hours =atoi(hours+2);
	if(mins != 0) ds_time.mins =atoi(mins+2);
	if(secs != 0) ds_time.secs =atoi(secs+2);

	printf("fix: 20%d.%d.%d	%d:%d:%d\n", ds_time.years, 	ds_time.month, ds_time.date, ds_time.hours, ds_time.mins, ds_time.secs);

	ds1302_write(ds_time);

	memset(recv_val, 0, BUF_SIZE);
}

void ds1302_main(void){
#ifdef DA1302_RESET
	ds_time.years = 25;
	ds_time.month = 12;
	ds_time.date = 17;
	ds_time.hours = 16;
	ds_time.mins = 27;
	ds_time.secs = 00;

	ds1302_write(ds_time);
#endif // DA1302_RESET


	while(1){
		if(recv_val[0] != 0){
			recv_parsing();
		}
		ds1302_read(&ds_time);

		printf("20%d.%d.%d	%d:%d:%d\n", ds_time.years, 	ds_time.month, ds_time.date, ds_time.hours, ds_time.mins, ds_time.secs);

		HAL_Delay(1000);
	}
}



