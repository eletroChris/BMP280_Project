## BMP280 Temperature and Pressure Visualizer

This is a simple project that aims to display on the computer screen the temperature and pressure in real time. 

The code can be tweaked to build larger projects like a weather station, for example. 

We are using a BMP280 pressure and temperature sensor, a Nrf52840 development kit and a python script to plot real time graphs. 

The Nrf52840 uses a I2C interface to communicate with the sensor. The data is processed in the board and sent to the computer via serial communication. A python script gets the data and updates the graphs.

BMP280 Sensor:
![sensor](/Datasheets_and_Images/sensorBMP280.jpg )

### Setting Up The Devices

This repository has all the files necessary to build and flash the firmware to the Nrf52840. I recommend using the nrF Connect extension for VScode. You can find the necessary information on how to configure it here: https://www.nordicsemi.com/Products/Development-hardware/nRF52840-DK/GetStarted

After you finish flashing the firmware, connect the sensor ports:

| BMP280 | Nrf52840 |
| ------------|--------- |
|Vcc pin      | 3.3 V pin |
|GND pin      | GND pin |
|SCL pin      | pin 14 |
|SDA pin      | pin 15 |

Board connections:
![board](/Datasheets_and_Images/board.jpg)

Run the python script plotSensorData.py inside src folder. You should be able to se the temperature and sensor plots:

![graphs](/Datasheets_and_Images/screenGif.gif)