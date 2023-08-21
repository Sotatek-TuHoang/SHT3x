# ESP32 IoT Project with SHT3x Sensor, Smart Config, and MQTT

## Introduction

This project demonstrates an IoT application using the ESP32 microcontroller, ESP-IDF framework, and various components to collect data from the SHT3x temperature and humidity sensor. The collected data is then sent to an MQTT server for remote monitoring and control.

## Features

- Read temperature and humidity data from the SHT3x sensor.
- Send collected data to an MQTT server for remote monitoring.
- Utilize Smart Config (SofAP or BLE) for convenient WiFi configuration.
- Configure WiFi settings using GPIO.
- Designed for ESP32 development boards.
- Requires the ESP-IDF Framework to be installed.
- Integrates SHT3x temperature and humidity sensor.
- MQTT server to receive and transmit data.
- Developed with ESP-IDF development environment and tools.

## Installation and Configuration

1. Ensure you have the ESP-IDF 5.1 framework installed.

2. Navigate to the project directory and run the following commands:

- idf.py fullclean to clean the project.
- idf.py build to build the project.
- idf.py flash to flash the firmware to the ESP32.
- idf.py monitor to start monitoring the device.
- Configure the MQTT server address and connection parameters in the "bee_mqtt.h" file if needed.

3. Adjust the power-saving mode settings in the "bee_deep_sleep.c" file if necessary.

## Additional Resources

For detailed technical specifications and information about the SHT3x temperature and humidity sensor, refer to the official [SHT3x Datasheet](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf).

## Usage

1. Compile and upload the project to an ESP32 development kit.

2. The ESP32 will collect temperature and humidity data from the SHT3x sensor and send it to the MQTT server.

3. Monitor temperature and humidity data remotely using the MQTT server.

4. Configure WiFi settings using GPIO button.

## Important Note

This project serves as a foundation for building IoT applications. Ensure that you review and customize the code to suit your specific use case and requirements.

## Credits
SHT3x lib was created with inspiration from the [sht3x-esp-idf](https://github.com/gschorcht/sht3x-esp-idf) repository. It is provided as-is without any warranty. Feel free to modify and extend it to match your project needs.

