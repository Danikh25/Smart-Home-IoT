# Smart Home
IoT-powered smart kitchen, featuring an Automation and Monitoring system
The main idea is to implement a desired and controlled action when a person exits or enters their bed, the actions being triggered when certain conditions are validated. The conditions will be discussed in a later section. The system uses motion sensors placed in the areas where a person would place their feet when exiting/entering the bed to detect the occupancy status of the bed. Based on the sensor data collected, a gateway device will make real-time decisions to control the lighting and activate a coffee machine, assuming conditions were met.

## Overall architecture of the project
![image](https://github.com/Danikh25/Smart-home-/assets/91975571/f3ff9244-9e2e-4a06-81bc-04eb9582e786)

## Hardware Used:
* Microcontroller: GK-ESP32-WROOM-32 from geeekus
  * This microprocessor supports Wi-Fi (802.11 b/g/n) and Bluetooth (Classic and Low Energy). It is in charge of powering and regulating the motion sensor acting as the publisher in the MQTT protocol
    
* Motion Sensor
  * This sensor is intended to be positioned on the bedside where the individual typically positions their feet when getting out of bed in the morning
    
* Smart Plug
  * A smart plug is a device that can be plugged into a standard electrical outlet, allowing it to control the power to any appliance plugged in using the ESP32


## Software Used:
* Arduino IDE
  * Publisher Code
  * Subscriber Code
    
* MQTT Mosquitto broker
  *  Port 1884 was then implemented for an added layer of security as it offers authentication making sure only the right people have access
    
* Firebase
  * Helped us visualize  real-time data synchronization of our motion state, a critical aspect of our application's functionality
