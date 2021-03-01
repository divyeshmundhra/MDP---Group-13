const { EventEmitter } = require("events");
const { spawn } = require("child_process");
const SerialPort = require("./SerialPort.js");

class Controller extends EventEmitter {
  constructor(serialPortPath) {
    super();

    this.logger = require("./logger.js")(`controller-${serialPortPath}`);
    this.sp = new SerialPort(serialPortPath);
    this.sp.on("beforeopen", () => {
      // the serial port opened by rfcomm does not close properly when the bluetooth connection terminates
      // helpfully, since `rfcomm listen hci0` terminates when the bluetooth connection closes,
      // we can kill the serial port when this process exits
      const logger = require("./logger.js")("rfcomm");
      this.logger = logger;
      // stdbuf to disable output buffering of rfcomm
      // https://unix.stackexchange.com/a/25378
      const rfcomm = spawn(
        "sudo",
        "stdbuf -oL -eL rfcomm listen hci0".split(" ")
      );

      rfcomm.stdout.on("data", (data) => {
        logger.info(data.toString().trim());
      });

      rfcomm.stderr.on("data", (data) => {
        logger.error(data.toString().trim());
      });

      rfcomm.on("error", (err) => {
        logger.error(`process error: ${err}`);
      });

      rfcomm.on("close", (code) => {
        if (code !== 0) {
          logger.info(`process exit with code ${code}`);
        }
        this.sp.close();
      });
    });

    this.sp.on("data", (line) => this._onSerialData(line));

    this.sp.open();
  }

  _onSerialData(line) {
    this.emit("data", line.trim());
  }

  send(data) {
    this.sp.send(data.trim() + "\n");
  }
}

module.exports = Controller;
