
#Raspbian Buster, OpenCV 4.1.0 pip installation
#TensorFlow Lite 2.5.0.
#Program structure: image recognition, servo controlling, object tracking 
#Software use deep learning detection to recogniction object class
#TFLite coco mobileNet quant model for mobile devices
#Real time video streaming
#Video capturing on the individual thread
#Camera tracks object and positioning it on the frame center
#Servos position are saved in external file 
#FPS function to calculate fps
#FPS score 2,4 FPS
#Csv file store presence, date, time, and position information
#Servos controll is proportional movmement to distance between center of the screen and object center

import os
import cv2
import time
import datetime
import pandas as pd
import numpy as np

from threading import Thread
from tflite_runtime.interpreter import Interpreter
from imutils.video import FPS

from board import SCL, SDA
import busio

from adafruit_pca9685 import PCA9685
from adafruit_motor import servo

# function to save data about object in dataframe
def SaveData(p, x, y, d, t, df):
    new_row = {"presence":p, "servo x":x, "servo y":y, "Date":d, "Time":t}
    df = df.append(new_row, ignore_index=True)
    return df

# function to setting servos position
def Movement(x_medium, centerX, y_medium, centerY, pan, tilt):

	#distane between screen center and object center
    DistanceX = abs(x_medium - centerX)
    DistanceY = abs(y_medium - centerY)

    propX = int(DistanceX / 20)
    propY = int(DistanceY / 20)

    if x_medium < centerX - 10:
        pan = pan + propX
        if pan >= 180:
            pan = 180
    elif x_medium > centerX + 10:
        pan = pan - propX
        if pan <= 0:
            pan = 0

    if y_medium > centerY - 10:
        tilt = tilt + propY
        if tilt >= 180:
            tilt = 180
    elif y_medium < centerY + 10:
        tilt = tilt - propY
        if tilt <= 0:
            tilt = 0

    return pan, tilt

# Class to capture video on individual thread
class VideoStream:

    def __init__(self, resolution=(640, 480)):
        # init Pi camera
        self.stream = cv2.VideoCapture(0)
        self.stream.set(3, resolution[0])
        self.stream.set(4, resolution[1])

        # read first frame
        (self.grabbed, self.frame) = self.stream.read()

        # variable when capture is stopped
        self.stopped = False

    def start(self):
        # Start the thread that reads frames from the video stream
        Thread(target=self.update, args=()).start()
        return self

    def update(self):
	    # infinite loop until thread is stopped
        while True:
            # if the camera is stopped, stop the thread
            if self.stopped:
                # close camera resources
                self.stream.release()
                return

            # otherwise, grab the next frame from the stream
            (self.grabbed, self.frame) = self.stream.read()

    def read(self):
        # return the most recent frame
        return self.frame

    def stop(self):
        # indicate that the camera and thread should be stopped
        self.stopped = True

# init i2C and servo controller
i2c = busio.I2C(SCL, SDA)
pca = PCA9685(i2c)
pca.frequency = 50

delay = 0         # variable that indicates time delay 
presence = False  # person detection tag
action = False    # action tedection tag
tstart = 0
# servos assing to controller
servo_pan = servo.Servo(pca.channels[8], min_pulse=350, max_pulse=2400)
servo_tilt = servo.Servo(pca.channels[11], min_pulse=350, max_pulse=2400)

# dataframe to archive info about object
data = {"presence":[], 'servo x':[], 'servo y':[], 'Date':"", 'Time':"" }
df = pd.DataFrame(data)

# read the last servo positions
with open("pan_tilt.txt", "r") as file:
    pan = int(file.readline())
    tilt = int(file.readline())
servo_pan.angle = pan
servo_tilt.angle = tilt

# set the model and parameters 
MODEL_NAME = "coco_ssd_mobilenet_v1"
GRAPH_NAME = "detect.tflite"
LABELMAP_NAME = "labelmap.txt"
min_conf_threshold = 0.5
resW, resH = 640, 480

y_medium =int(resH / 2)     # frame center
x_medium = int(resW / 2)
centerX = int(resW / 2)     # object center
centerY = int(resH / 2)

# path to current working directory,.tflite file, which contains the model that is used for object detection 
CWD_PATH = os.getcwd()
PATH_TO_CKPT = os.path.join(CWD_PATH, MODEL_NAME, GRAPH_NAME)
PATH_TO_LABELS = os.path.join(CWD_PATH, MODEL_NAME, LABELMAP_NAME)

# load the lable map
with open(PATH_TO_LABELS, 'r') as f:
    labels = [line.strip() for line in f.readlines()]

# first lable has to be removed
if labels[0] == '???':
    del (labels[0])

interpreter = Interpreter(model_path=PATH_TO_CKPT) # load the Tensorflow Lite model.
interpreter.allocate_tensors()

# get model details
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()
height = input_details[0]['shape'][1]
width = input_details[0]['shape'][2]

# init fps calculator
frame_rate_calc = 1
freq = cv2.getTickFrequency()

# video stream init
videostream = VideoStream(resolution=(resW, resH)).start()
time.sleep(1)

# create window
cv2.namedWindow('Person tracking detector', cv2.WINDOW_NORMAL)

fps = FPS().start()

# main loop real time processing
while True:

    # current date and time
    x = datetime.datetime.now()
    Date = x.strftime("%x")
    Time = x.strftime("%H:%M:%S")

    # labels display on the screen
    servo_label = '%s %d%s %d%s' % ("POS:", pan, "x", tilt, "y")
    Date_label = '%s %s %s %s' % ("DATE:", Date, "TIME:", Time)

    # start fps counter
    t1 = cv2.getTickCount()

    # frame from video
    frame1 = videostream.read()

    # acquire frame and resize to expected shape [1xHxWx3]
    frame = frame1.copy()
    frame_rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    frame_resized = cv2.resize(frame_rgb, (width, height))
    input_data = np.expand_dims(frame_resized, axis=0)

    # perform the actual detection by running the model with the image as input
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()

    # detection results
    boxes = interpreter.get_tensor(output_details[0]['index'])[0]    # bounding box coordinates of detected objects
    classes = interpreter.get_tensor(output_details[1]['index'])[0]  # class index of detected objects
    scores = interpreter.get_tensor(output_details[2]['index'])[0]   # confidence of detected objects

    # loop over all detections and draw detection box if confidence is above minimum threshold
    for i in range(len(scores)):

        # condition to extract only person detection
        if classes[i] != 0:
            scores[i] = 0

        if (scores[i] > min_conf_threshold) and (scores[i] <= 1.0):

            ymin = int(max(1, (boxes[i][0] * resH)))
            xmin = int(max(1, (boxes[i][1] * resW)))
            ymax = int(min(resH, (boxes[i][2] * resH)))
            xmax = int(min(resW, (boxes[i][3] * resW)))

            # get bounding box coordinates and draw box
            cv2.rectangle(frame, (xmin, ymin), (xmax, ymax), (10, 255, 0), 2)
            object_name = labels[int(classes[i])]
            label = '%s: %d%%' % (object_name, int(scores[i] * 100))

            # point at the screen center
            x_medium = xmin + (int(round((xmax - xmin) / 2)))
            y_medium = ymin + (int(round((ymax - ymin) / 2)))
            cv2.circle(frame, (x_medium, y_medium), 5, (0, 0, 255), thickness=2)

            # servos movement to track object 
            pan, tilt = Movement(x_medium, centerX, y_medium, centerY, pan, tilt)
            servo_pan.angle = pan
            servo_tilt.angle = tilt

            time_presence = cv2.getTickCount()
            presence = True     # person detection tag

            # center of object info
            center = '%s: %s:' % (x_medium, y_medium)
            cv2.putText(frame, center,(x_medium+10, y_medium+10), cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255, 255, 255), 2)
            cv2.putText(frame, label, (x_medium+10, y_medium - 30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (10, 255, 0), 2)

            # position and time labels 
            servo_label = '%s %d%s %d%s' % ("POS:", pan, "x", tilt, "y")
            Date_label = '%s %s %s %s' % ("DATE:", Date, "TIME:", Time)

    # fps, position and time labels 
    cv2.putText(frame, 'FPS: {0:.2f}'.format(frame_rate_calc), (15, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 2,
                cv2.LINE_AA)
    cv2.putText(frame, servo_label, (380, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 0), 2, cv2.LINE_AA)
    cv2.putText(frame, Date_label, (15, 465), cv2.FONT_HERSHEY_SIMPLEX, 0.75, (0, 255, 0), 2, cv2.LINE_AA)

    # display all results on the screen
    cv2.imshow('Person tracking detector', frame)

    # calculate fps and duration
    t2 = cv2.getTickCount()
    time1 = (t2 - t1) / freq
    frame_rate_calc = 1 / time1
    delay = delay + time1
    time3 = (t2 - time_presence) /freq

    # 'q' quit program 'a' symulate action (grab location)
    k = cv2.waitKey(1)
    if k == ord('q'):
        break
    if k == ord('a'):
        action = True

    # if action was triggered or time elapsed save current data
    if action or delay > 10:
        delay = 0
        action = False
        df = SaveData(presence, pan, tilt, Date, Time, df)

    if time3 > 2:
        presence=False

    fps.update()

fps.stop()

# save the last servos position
with open("pan_tilt.txt", "w") as file:
    file.write(str(pan) + "\n")
    file.write(str(tilt) + "\n")

# add data to file
with open('dataset.scv', 'a+') as f:
    df.to_csv(f, header=f.tell()==0, encoding='utf-8', index=False)

print("[INFO] elapsed time: {:.2f}".format(fps.elapsed()))
print("[INFO] approx. FPS: {:.2f}".format(fps.fps()))

# clean all processes
cv2.destroyAllWindows()
videostream.stop()
pca.deinit()