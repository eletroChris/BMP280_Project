#include "bmp280.h"


int32_t t_fine;

int deviceChecking(const struct i2c_dt_spec * dev_i2c){
    
    if (!device_is_ready(dev_i2c->bus)) {
        printk("I2C bus %s is not ready!\n", dev_i2c->bus->name);
        return -1;
    }
    
    //Check device ID and chip ID
    
    uint8_t id = 0;
    uint8_t regs[] = {ID};
    
    int ret = i2c_write_read_dt(dev_i2c, regs, 1, &id, 1);
    
    if (ret != 0) {
        printk("Failed to read register %x \n", regs[0]);
        return -1;
    }
    
    if (id != CHIP_ID) {
        printk("Invalid chip id! %x \n", id);
        return -1;
    }
    return 0;
}


int deviceConfiguration(const struct i2c_dt_spec * dev_i2c){
    uint8_t sensor_config[] = {CTRLMEAS, SENSOR_CONFIG_VALUE};

	int ret = i2c_write_dt(dev_i2c, sensor_config, 2);

	if (ret != 0) {
		printk("Failed to write register %x \n", sensor_config[0]);
		return -1;
	}

	uint8_t fir_standby_config[] = {CONFIG_REG, STANDBY_AND_FIR};

	ret = i2c_write_dt(dev_i2c, fir_standby_config, 2);

	if (ret != 0) {
		printk("Failed to write register %x \n", fir_standby_config[0]);
		return -1;
	}

    return 0;
}

void bmp280_calibrationdata(const struct i2c_dt_spec *spec, bmp280data *data)
{
	
	uint8_t values[24];

	int ret = i2c_burst_read_dt(spec, CALIB00, values, 24);

	if (ret != 0) {
		printk("Failed to read register %x \n", CALIB00);
		return;
	}

	data->dig_t1 = ((uint16_t)values[1]) << 8 | values[0];
	data->dig_t2 = ((int16_t)values[3]) << 8 | values[2];
	data->dig_t3 = ((int16_t)values[5]) << 8 | values[4];

    data->dig_P1 = ((uint16_t)values[7]) << 8 | values[6];
    data->dig_P2 = ((int16_t)values[9]) << 8 | values[8];
    data->dig_P3 = ((int16_t)values[11]) << 8 | values[10];
    data->dig_P4 = ((int16_t)values[13]) << 8 | values[12];
    data->dig_P5 = ((int16_t)values[15]) << 8 | values[14];
    data->dig_P6 = ((int16_t)values[17]) << 8 | values[16];
    data->dig_P7 = ((int16_t)values[19]) << 8 | values[18];
    data->dig_P8 = ((int16_t)values[21]) << 8 | values[20];
    data->dig_P9 = ((int16_t)values[23]) << 8 | values[22];
}

int32_t bmp280_compensate_temp(bmp280data *data, int32_t adc_temp)
{
	int32_t var1, var2;
    

	var1 = (((adc_temp >> 3) - ((int32_t)data->dig_t1 << 1)) * ((int32_t)data->dig_t2)) >> 11;

	var2 = (((((adc_temp >> 4) - ((int32_t)data->dig_t1)) *
		  ((adc_temp >> 4) - ((int32_t)data->dig_t1))) >>
		 12) *
		((int32_t)data->dig_t3)) >>
	       14;
    t_fine = var1 + var2;
	return ((var1 + var2) * 5 + 128) >> 8;
}

int32_t bmp280_compensate_press(bmp280data *data, int32_t adc_press){
    int32_t var1, var2;
    
    uint32_t p;
    var1 = ((t_fine)>>1) - (int32_t)64000;
    var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((int32_t)data->dig_P6);
    var2 = var2 + ((var1*((int32_t)data->dig_P5))<<1);
    var2 = (var2>>2)+(((int32_t)data->dig_P4)<<16);
    var1 = (((data->dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((int32_t)data->dig_P2) * var1)>>1))>>18;
    var1 =((((32768+var1))*((int32_t)data->dig_P1))>>15);
    if (var1 == 0){
        return 0; // avoid exception caused by division by zero
    }
    p = (((uint32_t)(((int32_t)1048576)-adc_press)-(var2>>12)))*3125;
    if (p < 0x80000000){
        p = (p << 1) / ((uint32_t)var1);
    }
    else{
        p = (p / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)data->dig_P9) * ((int32_t)(((p>>3) * (p>>3))>>13)))>>12;
    var2 = (((int32_t)(p>>2)) * ((int32_t)data->dig_P8))>>13;
    p = (uint32_t)((int32_t)p + ((var1 + var2 + data->dig_P7) >> 4));
    return p;
}
//BMP280_U32_t = uint32_t
//BMP280_S32_t = int32_t
// BMP280_U32_t bmp280_compensate_P_int32(BMP280_S32_t adc_P)
// {
// BMP280_S32_t var1, var2;
// BMP280_U32_t p;
// var1 = (((BMP280_S32_t)t_fine)>>1) – (BMP280_S32_t)64000;
// var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BMP280_S32_t)dig_P6);
// var2 = var2 + ((var1*((BMP280_S32_t)dig_P5))<<1);
// var2 = (var2>>2)+(((BMP280_S32_t)dig_P4)<<16);
// var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BMP280_S32_t)dig_P2) * var1)>>1))>>18;
// var1 =((((32768+var1))*((BMP280_S32_t)dig_P1))>>15);
// if (var1 == 0)
// {
// return 0; // avoid exception caused by division by zero
// }
// p = (((BMP280_U32_t)(((BMP280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
// if (p < 0x80000000)
// {
// p = (p << 1) / ((BMP280_U32_t)var1);
// }
// else
// {
// p = (p / (BMP280_U32_t)var1) * 2;
// }
// var1 = (((BMP280_S32_t)dig_P9) * ((BMP280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
// var2 = (((BMP280_S32_t)(p>>2)) * ((BMP280_S32_t)dig_P8))>>13;
// p = (BMP280_U32_t)((BMP280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
// return p;
// }