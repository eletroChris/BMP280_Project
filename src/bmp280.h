
#ifndef BMP280_H
#define BMP280_H

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>

//Important Registers
#define CTRLMEAS 0xF4
#define CALIB00	 0x88
#define ID	 0xD0
#define TEMPMSB	 0xFA
#define PRESSMSB	 0xF7
#define CONFIG_REG 0xF5


//Configuration Values
#define CHIP_ID  0x58
#define SENSOR_CONFIG_VALUE 0x2F
#define STANDBY_AND_FIR 0x0

typedef struct  {
	// Temperature Compensation Parameters
	uint16_t dig_t1;
	int16_t dig_t2;
	int16_t dig_t3;
    // Pressure Compensation Parameters
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
}bmp280data;

 //Fine Temperature value used for pressure compensation

int deviceChecking(const struct i2c_dt_spec * dev_i2c);
int deviceConfiguration(const struct i2c_dt_spec * dev_i2c);
void bmp280_calibrationdata(const struct i2c_dt_spec *spec, bmp280data *data);
int32_t bmp280_compensate_temp(bmp280data *data, int32_t adc_temp);

#endif 