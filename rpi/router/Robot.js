const { EventEmitter } = require("events");
const SerialPort = require("./SerialPort.js");

class Robot extends EventEmitter {
  constructor(serialPortPath, baudRate) {
    super();

    this.logger = require("./logger.js")(`robot-${serialPortPath}`);
    this.sp = new SerialPort(serialPortPath, baudRate);
    this.sp.open();

    this.sp.on("data", this._onSerialData);
  }

  _onSerialData(line) {
    let match = null;

    if (
      (match = line.match(
        /\$SENSOR ([0-9]+)\|([0-9]+)\|([0-9]+)\|([0-9]+)\|([0-9]+)\|([0-9]+)\|/
      ))
    ) {
      // match `$SENSOR 0|1|2|3|4|5|`
      // sensor update
      const SENSOR_MAP = [
        "LEFT_FRONT",
        "FRONT_FRONT_LEFT",
        "FRONT_FRONT_MID",
        "FRONT_FRONT_RIGHT",
        "LEFT_REAR",
        "RIGHT_FRONT",
      ];
      const update = {};
      for (const i in SENSOR_MAP) {
        let val = false;
        if (match[i] !== "i") {
          val = parseInt(match[i], 10);
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
