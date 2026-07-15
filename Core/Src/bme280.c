#include <stdio.h>
#include "main.h"
#include "bem.h"

extern I2C_HandleTypeDef hi2c1;

bme280_calib_t cal;
static int32_t t_fine;
int32_t bme280_compensate_T(int32_t adc_T)
{
    int32_t var1, var2, T;
    var1 = ((((adc_T >> 3) - ((int32_t)cal.dig_T1 << 1))) * ((int32_t)cal.dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)cal.dig_T1)) * ((adc_T >> 4) - ((int32_t)cal.dig_T1))) >> 12) *
            ((int32_t)cal.dig_T3)) >> 14;
    t_fine = var1 + var2;
    T = (t_fine * 5 + 128) >> 8;
    return T;   /* in 0.01 °C — 2643 means 26.43 °C */
}

bool bme280init()
{
	uint8_t calib1[26];
	uint8_t calib2[7];
	uint8_t v;

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
	v = 0x01;
	if(HAL_I2C_Mem_Write(&hi2c1, 0X76<<1, 0XF2, I2C_MEMADD_SIZE_8BIT, &v, 1, 100)!=HAL_OK)
	{
		return false;
	}
	v = 0x27;
	if(HAL_I2C_Mem_Write(&hi2c1, 0X76<<1, 0XF4, I2C_MEMADD_SIZE_8BIT, &v, 1, 100)!=HAL_OK)
	{
		return false;
	}
	return true;

}

bool bme28ReadRaw(int32_t *raw_t)
{
	uint8_t raw[8];
	HAL_I2C_Mem_Read(&hi2c1, 0X76<<1, 0XF7, I2C_MEMADD_SIZE_8BIT, raw, 8, 100);
	*raw_t = ((int32_t)raw[3] <<12) | ((int32_t)raw[4] << 4) | (raw[5] >>4);
}


