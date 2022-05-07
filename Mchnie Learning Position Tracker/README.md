# Machine Learning Position Tracker 

The main assumption of this project was designing  monitoring system which allow to define position and location in the room. It is possible by apply immage processing with machine learning and combinr it with pan tilt camera system.

## Technology:
* Raspberry PI
* Python
* OpenCV
* TensorFlow Lite

## Application:
System consist of Raspberry Pi processing unit with cooling system, pan tilt with two servos, servos controler and HD camera. System enables to controling camera in two planes with range in 180 deggres. 
Open CV and TensorFlow library process image to recognise person on the video capture and servo controlling algorithm holds object in the center on the frame. Servos position signal in deegre unit indirectly determine what is person location in the room. Data is stored in csv file. Every data point is captue based on sampling time and presence recognition.  


## Specificaton:
* Hardware:
    * Raspberry Pi 3B+
    * servo controller PCA9685
    * Arducam OV5647
    * Pan tilt mechanism
    
* Software:
    * Pesron recognition 
    * Object tracking
    * data storage

* performance:
    * image processing in 2.5 FPS

---
## Design:
* ### Source Code:

* ### Circuit Diagram:
<img src="" width="600">

* ### Data Example:
<img src="" width="600">

* ### Frame Example:
<img src="" width="600">

* ### Assembled device:
<img src="" width="600">
