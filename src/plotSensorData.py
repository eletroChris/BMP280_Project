import serial
import time
import re
import matplotlib.pyplot as plt



MAX_DATA_POINTS = 100
FILTER_POINTS = 10

# Replace with your serial port name
SERIAL_PORT = 'COM4'      # Windows example: COM3, Linux/Mac: '/dev/ttyUSB0'
BAUD_RATE = 115200         # Match your device's baud rate

averagePressFilter = []
averageTempFilter = []

#Data Buffers
temp_data = []
press_data = []
time_data = []

# Plot setup
plt.ion()  # turning interactive mode on
figure,axis = plt.subplots(1,2)

def averageFilter(temp,press):
    if(len(averagePressFilter) == FILTER_POINTS):
        averagePressFilter.pop(0)
        averageTempFilter.pop(0)
    
    averagePressFilter.append(press)
    averageTempFilter.append(temp)

    if(len(averagePressFilter)<FILTER_POINTS): #Not enough data to fill the filter window. We return this default -999 value
        return (-999,-999)
    else:
        meanPress = sum(averagePressFilter)/FILTER_POINTS
        tempPress = sum(averageTempFilter)/FILTER_POINTS
        return (meanPress,tempPress)

def updatePlot(temp,pressure,time,figure,axis):
    figure.remove

    #Adding new entries to the buffer
    temp_data.append(temp)
    press_data.append(pressure)
    time_data.append(time)

    #Cleaning older entries from the buffer
    if len(temp_data) > MAX_DATA_POINTS:
        temp_data.pop(0)
        time_data.pop(0)
        press_data.pop(0)
    
    #Setting Temperature Plot
    axis[0].plot(time_data,temp_data,'b')   
    axis[0].set_title("Temperature (°C) x Time")
    axis[0].set_xlim(time_data[0], time_data[-1])
    axis[0].set_ylim(min(temp_data)-0.5,max(temp_data)+0.5)
    axis[0].locator_params(axis='y', nbins=10)
    axis[0].locator_params(axis='x', nbins=10)
    axis[0].grid(color='k', linestyle='-', linewidth=0.5)
    axis[0].set_ylabel("Temperature")

    #Setting Pressure Plot
    axis[1].plot(time_data,press_data,'r')   
    axis[1].set_title("Pressure (mmHg) x Time")
    axis[1].set_xlim(time_data[0], time_data[-1])
    axis[1].set_ylim(min(press_data)-0.5,max(press_data)+0.5)
    axis[1].locator_params(axis='y', nbins=10)
    axis[1].locator_params(axis='x', nbins=10)
    axis[1].grid(color='k', linestyle='-', linewidth=0.5)
    axis[1].set_ylabel("Pressure")
    plt.pause(0.25)

try:
    # Open the serial connection
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    time.sleep(2)  # Wait for connection to establish

    print(f"Reading from {SERIAL_PORT} at {BAUD_RATE} baud... (Press Ctrl+C to stop)")
    start_time = time.time()
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()            
            ser.reset_input_buffer()
            pattern = r"[-+]?\d+\.\d{2}" #Catching decimal numbers from serial
            data = re.findall(pattern, line)
            
            print(f"Received: {line} - Filtered by Regex: {data}") # Printing for debug purpuses

            #This condition discards incomplete data coming from serial. Temperature and pressure
            # should come in pairs
            if(len(data)==2):
                temperature = float(data[0])            
                pressure = float(data[1])            
                
                #Associating time to measure
                end_time = time.time()
                elapsed = end_time - start_time
                meanTempPress = averageFilter(temperature,pressure) # Returns a tuple with average pressure and temperature e.g. (960,25)
                if(meanTempPress[0]!= -999):
                    updatePlot(meanTempPress[1],meanTempPress[0],elapsed,figure,axis)
            

except serial.SerialException as e:
    print(f"Error: {e}")
except KeyboardInterrupt:
    print("\nStopped by user.")
finally:
    if 'ser' in locals() and ser.is_open:
        ser.close()
        print("Serial port closed.")