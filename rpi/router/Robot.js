const { EventEmitter } = require("events");
const SerialPort = require("./SerialPort.js");

class Robot extends EventEmitter {
  constructor(serialPortPath, baudRate) {
    super();

    this.logger = require("./logger.js")(`robot-${serialPortPath}`);
    this.sp = new SerialPort(serialPortPath, baudRate);
    this.sp.open();

    this.sp.on("data", (line) => this._onSerialData(line));
  }

  _onSerialData(line) {
    let match = null;

    this.emit("data", line);

    if (
      (match = line.match(
        /\$SENSOR ([0-9i]+)\|([0-9i]+)\|([0-9i]+)\|([0-9i]+)\|([0-9i]+)\|([0-9i]+)\|/
      ))
    ) {
      // match `$SENSOR 0|1|2|3|4|5|`
      // sensor update
      const SENSOR_MAP = [
        "FORWARD_FRONT_MID",
        "FORWARD_FRONT_RIGHT",
        "LEFT_REAR",
        "FORWARD_FRONT_LEFT",
        "LEFT_FRONT",
        "RIGHT_FRONT",
      ];
      const update = {};
      for (const i in SENSOR_MAP) {
        let val = false;

        let offset = parseInt(i) + 1;
        if (match[offset] !== "i") {
          val = parseInt(match[offset], 10);
        }
        if (val > 6) {
          val = 6;
          this.logger.debug("throttling");
        }
        update[SENSOR_MAP[i]] = val;
      }

      this.logger.verbose(`Sensor: ${JSON.stringify(update)}`);
      this.emit("sensors", update);
    } else if ((match = line.match(/\$U([FBLR])/))) {
      const EVENT_MAP = {
        F: "forward",
        B: "backward",
        L: "left",
        R: "right",
      };
      const move = EVENT_MAP[match[1]];

      this.logger.verbose(`Move: ${move}`);
      this.emit("move", move);
    }
  }

  send(data) {
    this.sp.send(data.trim() + "\n");
  }
}

module.exports = Robot;
