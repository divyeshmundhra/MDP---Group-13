import zmq
import sys
import json
import time

context = zmq.Context()
rx = context.socket(zmq.SUB)
rx.connect("tcp://192.168.13.1:3000")

tx = context.socket(zmq.PUSH)
tx.connect("tcp://192.168.13.1:3001")

rx.setsockopt_string(zmq.SUBSCRIBE, '')

if len(sys.argv) < 2:
    sys.exit("Usage: send_cmd.py <json cmd>")

time.sleep(0.1)

tx.send_json({"type": "echo", "data": json.loads(sys.argv[1])})
tx.send_json({"type": "echo", "data": {"type":"sensor","data":{"FORWARD_FRONT_MID":False,"FORWARD_FRONT_RIGHT":False,"LEFT_REAR":False,"FORWARD_FRONT_LEFT":False,"LEFT_FRONT":False,"RIGHT_FRONT":False}}})
tx.send_json({"type": "echo", "data": {"type": "start"}})
