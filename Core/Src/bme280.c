#include <stdio.h>
#include "main.h"
#include "bem.h"

extern I2C_HandleTypeDef hi2c1;

static bme280_calib_t cal;

bool bme280init()
{
	uint8_t calib1[26];
	uint8_t calib2[7];

	if(HAL_I2C_Mem_Read(&hi2c1, 0X76<<1, 0X88, I2C_MEMADD_SIZE_8BIT, calib1, 26, 100) == HAL_OK)
	{
		for(uint8_t i=0; i<26; i++)
		{
			//printf(" 0x%02X  ", calib1[i]);
		}
		//printf("\r\n");
		cal.dig_T1 = (calib1[1] <<8 | calib1[0]);
		cal.dig_T2 = (calib1[3] <<8 | calib1[2]);
		cal.dig_T3 = (calib1[5] <<8 | calib1[4]);
		cal.dig_H1 = calib1[25];
		cal.dig_P1 = (calib1[7] << 8) | calib1[6];
		cal.dig_P2 = (calib1[9] << 8) | calib1[8];
		cal.dig_P3 = (calib1[11] << 8) | calib1[10];
		cal.dig_P4 = (calib1[13] << 8) | calib1[12];
		cal.dig_P5 = (calib1[15] << 8) | calib1[14];
		cal.dig_P6 = (calib1[17] << 8) | calib1[16];
		cal.dig_P7 = (calib1[19] << 8) | calib1[18];
		cal.dig_P8 = (calib1[21] << 8) | calib1[20];
		cal.dig_P9 = (calib1[23] << 8) | calib1[22];
	}
	if(HAL_I2C_Mem_Read(&hi2c1, 0X76<<1, 0XE1, I2C_MEMADD_SIZE_8BIT, calib2, 7, 100) == HAL_OK)
	{
			cal.dig_H2 = calib2[1] << 8 | calib2[0];
			cal.dig_H3 = calib2[2];
			cal.dig_H4 = (calib2[3] << 4) | (calib2[4] & 0x0F);
			cal.dig_H5 = (calib2[5] << 4) | (calib2[4] >> 4);
			cal.dig_H6 = calib2[6];
	}
	else
	{
		printf("calib read FAILED\r\n");
		return false;
	}
    printf("BME280 init OK: T1=%u T2=%d P5=%d H1=%u\r\n",
           cal.dig_T1, cal.dig_T2, cal.dig_P5, cal.dig_H1);
	return true;

}
