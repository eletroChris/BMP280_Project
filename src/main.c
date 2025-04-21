#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/uart.h>
#include "bmp280.h"
/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS 100


#define I2C_NODE DT_NODELABEL(mysensor)

const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));


int main(void)
{
	// Transmission buffer
	char tx_buf[50];

	if (!device_is_ready(uart)) {
		printk("UART device not ready\n");
		return 1;
	}

	//Retrieve the API-specific device structure and make sure that the device is ready to use  
	static const struct i2c_dt_spec dev_i2c = I2C_DT_SPEC_GET(I2C_NODE);

	int ret = deviceChecking(&dev_i2c);
	if(ret!=0){
		return -1;
	}
	
	//Collect and store calibration parameters
	bmp280data bmp280_data;
	
	bmp280_calibrationdata(&dev_i2c, &bmp280_data);

	ret = deviceConfiguration(&dev_i2c);
	if(ret!=0){
		return -1;
	}
	
	while (1) {

		//Read the temperature from the sensor
		uint8_t temp_val[3] = {0};

		int ret = i2c_burst_read_dt(&dev_i2c, TEMPMSB, temp_val, 3);

		if (ret != 0) {
			printk("Failed to read Temperature register %x \n", TEMPMSB);
			k_msleep(SLEEP_TIME_MS);
			continue;
		}

		//Read the pressure from sensor
		uint8_t press_val[3] = {0};
		ret = i2c_burst_read_dt(&dev_i2c, PRESSMSB, press_val, 3);

		if (ret != 0) {
			printk("Failed to read Pressure register %x \n", PRESSMSB);
			k_msleep(SLEEP_TIME_MS);
			continue;
		}

		
		// Put the data read from registers into actual order (see datasheet)
		int32_t adc_temp =
			(temp_val[0] << 12) | (temp_val[1] << 4) | ((temp_val[2] >> 4) & 0x0F);

		int32_t adc_press =
			(press_val[0] << 12) | (press_val[1] << 4) | ((press_val[2] >> 4) & 0x0F);

		//Compensate temperature
		int32_t comp_temp = bmp280_compensate_temp(&bmp280_data, adc_temp);

		//Compensate pressure
		int32_t comp_press = bmp280_compensate_press(&bmp280_data, adc_press);

		// Convert temperature 
		float temperature = (float)comp_temp / 100.0f;

		// Convert pressure to mmHg
		float pressure = (float)comp_press * 0.00750062f;
		
		

		// Print reading to console. Useful for debugging
		//printk("Temperature in Celsius : %8.2f C\n", (double)temperature);
		//printk("Pressure : %8.2f mmHg\n", (double)pressure);
		
		memset(tx_buf,'\0',sizeof(tx_buf)); //Initialize buffer with empty string
		sprintf(tx_buf,"%8.2f|%8.2f\r\n",(double)temperature,(double)pressure);

		
		//Send data through serial
		ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_MS);
		if (ret) {
			return 1;
		}

		k_msleep(SLEEP_TIME_MS);
	}
}


