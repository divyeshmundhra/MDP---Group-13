import zmq

HOST = "192.168.13.1"
#HOST = "172.20.48.1"

context = zmq.Context()
rx = context.socket(zmq.SUB)
rx.connect(f"tcp://{HOST}:3000")

tx = context.socket(zmq.PUSH)
tx.connect(f"tcp://{HOST}:3001")

rx.setsockopt_string(zmq.SUBSCRIBE, '')
