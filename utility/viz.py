import sys
import serial
import struct
import threading
from collections import deque
import numpy as np
from matplotlib import pyplot as plt

SENSOR_LABELS = ["Front Mid", "Front Right", "Left Rear", "Front Left", "Left Front", "Right Front"]

MAX_BUFF_SIZE = 256
"""
h: int16_t
H: uint16_t
i: int32_t
I: uint32_t
"""
# motion
if len(sys.argv) >= 3:
    choice = int(sys.argv[2])
else:
    choice = 0

if choice == 0:
    VAR_TITLES = ["Encoder Left", "Encoder Right", "Power Left", "Power Right", "Left Front", "Left Rear", "State", "Align Type"]
    VARS = 'iihhhhBB'
elif choice == 1:
    VAR_TITLES = [SENSOR_LABELS[i] for i in range(6)]
    VARS = "H" * 6
elif choice == 2:
    VAR_TITLES = ["Wall Diff", "Sensor Front", "Sensor Rear", "Encoder Error"]
    VARS = "hhhh"
elif choice == 3:
    VAR_TITLES = ['Pulse Left', 'Pulse Right']
    VARS = 'HH'
IDENTIFIER = b"SYNC"

data = [np.zeros(1) for i in range(len(VARS))]

ser = serial.Serial(sys.argv[1], 115200, timeout=None)

def read():
    global data, ser
    while ser.is_open:
        line = ser.readline()
        if not line.startswith(IDENTIFIER):
            # if not log line, just print it
            try:
                print(line.decode("utf8"), end="")
            except UnicodeDecodeError:
                pass
            continue

        # total packet is expected to be of a fixed line
        # keep reading if less, this handles the case of the data containing a new line eg SYNC\n\x00\x00\x00\n
        while len(line) < len(IDENTIFIER) + struct.calcsize(VARS) + 1:
            line += ser.readline()

        in_data = struct.unpack_from(VARS, line, offset=len(IDENTIFIER))

        for i, val in enumerate(in_data):
            data[i] = np.append(data[i], val)[-MAX_BUFF_SIZE:]
    
    print("Serial closed")

def write():
    global ser

    while True:
        inp = input("> ").encode("utf8") + b'\n'
        ser.write(inp)

t = threading.Thread(target=read, daemon=True)
t.start()

t = threading.Thread(target=write, daemon=True)
t.start()

f, axes = plt.subplots(len(VARS), 1)
plt.subplots_adjust(left=0.1, right=0.9, top=0.95, bottom=0.05)

lines = []
for i in range(len(VARS)):
    line, = axes[i].plot(data[i])
    lines.append(line)
    axes[i].autoscale()
    axes[i].set_title(VAR_TITLES[i], x=0.5, y=0.6)
    axes[i].get_xaxis().set_visible(False)

plt.ion()
plt.show()

texts = [axes[i].text(0, 0, str(data[i][-1])) for i in range(len(VARS))]
while True:
    try:
        for i in range(len(VARS)):
            lines[i].set_data(np.arange(len(data[i])), data[i])
            axes[i].set_xlim(0, len(data[i]))
            if min(data[i]) != max(data[i]):
                axes[i].set_ylim(min(data[i]) - 1, max(data[i]) + 1)

            texts[i].set_position((len(data[i]), data[i][-1]))
            texts[i].set_text(str(data[i][-1]))

        try:
            plt.pause(0.05)
        except:
            sys.exit()
    except KeyboardInterrupt:
        sys.exit()
