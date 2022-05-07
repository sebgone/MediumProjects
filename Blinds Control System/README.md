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
- Bluetooth communication
- Sensor battery supply
- Controlling web page
- Limit switches 
- DC motor with metal gear
---
## Design:
* ### Source Code:
  * [source code](https://github.com/sebgone/MediumProjects/tree/sebgone-update/Blinds%20Control%20System/Source%20Code)  

* ### Block Scheme:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/scheme.png" width="600">

* ### Circuit Diagram:
&nbsp;
&nbsp;

  * Central module
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Diagrams/CentralModule.PNG" width="600">
  
&nbsp;
&nbsp;

  * Driver
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Diagrams/Driver.PNG" width="600">
  
&nbsp;
&nbsp;

  * Light Sensor
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Diagrams/LightSensor.PNG" width="600">

* ### Circuit Eagle schematic:
&nbsp;
&nbsp;

   * Light Sensor 
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Layout/LightSensor%20Schematic.png" width="600">
&nbsp;
&nbsp;

   * Motor Driver 
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Layout/MotorDriver%20Schematic.png" width="600">


* ### Circuit Eagle PCB layout:
&nbsp;
&nbsp;

  * Light Sensor
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Layout/LightSensorPCB.png" width="600">
&nbsp;
&nbsp;

  * Moto Driver
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/Layout/MotorDriver%20PCB.png" width="600">


* ### Web Page:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/page.png" width="400">

* ### Assembled device:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Blinds%20Control%20System/img1.jpg">
