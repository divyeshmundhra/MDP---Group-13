import sys
import zmq

if len(sys.argv) < 2:
    sys.exit(f'Usage: {sys.argv[0]} arena_string [host]')

sock = zmq.Context().socket(zmq.REQ)
if len(sys.argv) >= 3:
    host = sys.argv[2]
else:
    host = "192.168.13.1"

connstr = f"tcp://{host}:3002"
print(f"Connecting to {connstr}")
sock.connect(connstr)

sock.setsockopt(zmq.RCVTIMEO, 100)

sock.send_json({"type": "setfparena", "data": {"P2": sys.argv[1]}})
try:
    print(sock.recv().decode("utf8"))
except zmq.error.Again:
    print("Connection timed out")
