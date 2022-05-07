# Blinds Control System

The purpose of this project was designing and building a control system for monitoring and setting the amount of sunlight in the room. The major assumption was based on making use of wireless solutions to communicate between elements of control system. Its parts consist of regulator, driver element and sensors. The functioning comes to read data from light sensors compare this value with desired value and then applies the difference as a control signal to steer a roller blinds by DC motor. 

## Technology:
* Arduino 
* Ethernet Shield
* Bluetooth

## Application:
The system consist of four elements. Central Module is a master device which supervise data comunication beetwen DC controler and bluetooth sensors. 
User has accesss by a web browser to set a specific light level or read actual value. Sensors send readings to master and it calculte average. 
The calculated value is compared with desired value. Next, based on difference range DC controler get up or down blinds.    


## specificaton:
- Four elements of a system
- bluetooth communication
- Sensor battery supply
- Controlling web page
- limit switches 
- DC motor with metal gear

---
## Design:
* ### Source Code:

* ### Block Scheme:
<img src="" width="600">

* ### Circuit Diagram:
<img src="" width="600">

* ### Circuit Eagle schematic:
<img src="" width="700">

* ### Circuit Eagle PCB layout:
<img src="" width="600">

* ### Communication Test:
<img src="" width="600">

* ### Web Page:
<img src="" width="600">

* ### Assembled device:
<img src="" width="600">
