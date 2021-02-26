from imutils.video import VideoStream
import imagezmq
import simplejpeg
import time
import sys

if len(sys.argv) >= 2:
        frame_rate = int(sys.argv[1])
else:
        frame_rate = 5

sender = imagezmq.ImageSender(connect_to="tcp://*:5555", REQ_REP=False)

vs = VideoStream(usePiCamera=True, resolution=(640,480), exposure_mode="sports").start()
time.sleep(2.0)

while True:
        frame = vs.read()
        jpg_buffer = simplejpeg.encode_jpeg(frame, quality=100, colorspace='BGR')
        sender.send_jpg("pi13", jpg_buffer)
        time.sleep(1/frame_rate)
