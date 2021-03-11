from imutils.video import VideoStream
import imagezmq
import simplejpeg
import time
import sys
import zmq
import json

if len(sys.argv) >= 2:
        frame_rate = int(sys.argv[1])
else:
        frame_rate = 5

sender = imagezmq.ImageSender(connect_to="tcp://*:5555", REQ_REP=False)

context = zmq.Context()
rx = context.socket(zmq.SUB)
rx.connect(f"tcp://localhost:3000")
# subscribe to all events
rx.setsockopt_string(zmq.SUBSCRIBE, '')

print("Waiting for camera init")

vs = VideoStream(usePiCamera=True, resolution=(640,480), exposure_mode="sports").start()
time.sleep(2.0)

print("Started")

while True:
        data = rx.recv_json()

        if data["type"] != "robotinfo":
                continue

        robot_info = data["data"]

        print(f"rx location {robot_info}")

        frame = vs.read()
        jpg_buffer = simplejpeg.encode_jpeg(frame, quality=100, colorspace='BGR')
        sender.send_jpg(json.dumps(robot_info), jpg_buffer)
