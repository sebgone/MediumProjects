# Machine Learning Position Tracker 

The main assumption of this project was designing  monitoring system which allow to define position and location in the room. It is possible by apply immage processing with machine learning and combinr it with pan tilt camera system.

## Technology:
* Raspberry PI
* Python
* OpenCV
* TensorFlow Lite

## Application:
System consist of Raspberry Pi processing unit with cooling system, pan tilt with two servos, servos controler and HD camera. System enables to controling camera in two planes with range in 180 deggres. 
Open CV and TensorFlow library process image to recognise person on the video capture and servo controlling algorithm holds object in the center on the frame. Servos position signal in deegre unit indirectly determine what is person location in the room. Data is stored in csv file. Every data point is capture based on sampling time and presence recognition.  


## Specificaton:
* Hardware:
    * Raspberry Pi 3B+
    * Servo controller PCA9685
    * Arducam OV5647
    * Pan tilt mechanism
    
* Software:
    * Pesron recognition 
    * Object tracking
    * Data storage

* performance:
    * Image processing in 2.5 FPS

---
## Design:
* ### Source Code:
   * [source code](https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/TFLite_detection_webcam.py)  

* ### Circuit Diagram:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/PanTilt.png" width="600">

* ### Data Example:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/data.png">

* ### Frame Example:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/photos/img3.png" width="600">

* ### Assembled device:
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/photos/img1.jpg" width="600">
<img src="https://github.com/sebgone/MediumProjects/blob/sebgone-update/Mchnie%20Learning%20Position%20Tracker/photos/img2.jpg" width="600">
