const { EventEmitter } = require("events");
const sp = require("serialport");
const Readline = require("@serialport/parser-readline");

const PORT_OPEN_RETRY_INTERVAL = 1000;

class SerialPort extends EventEmitter {
  constructor(path, baudRate) {
    super();

    if (typeof baudRate === "undefined") {
      baudRate = 115200;
    }

    this.logger = require("./logger.js")(path);

    this.sp = new sp(path, {
      baudRate,
      autoOpen: false,
    });

    this.sp.on("open", () => {
      this.logger.info("Port opened");
    });

    this.sp.on("close", () => {
      this.logger.info("Port closed");
      this.open();
    });

    this.sp.on("error", (err) => {
      if (err.toString().includes("Port is not open")) return;
      this.logger.error(err);
    });

    const parser = this.sp.pipe(new Readline());
    parser.on("data", (line) => {
      this.logger.verbose(`rx data: ${line}`);
      this.emit("data", line);
    });
  }

  open() {
    this.emit("beforeopen");

    const interval = setInterval(() => {
      this.sp.open((err) => {
        if (err) {
          if (err.toString().includes("No such file or directory")) return;
          this.logger.error(`port open: ${err}`);
          return;
        }

        clearInterval(interval);
      });
    }, PORT_OPEN_RETRY_INTERVAL);
  }

  close() {
    this.sp.close();
  }
}

module.exports = SerialPort;
