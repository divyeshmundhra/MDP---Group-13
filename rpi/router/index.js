const SerialPort = require("./SerialPort.js");
const { spawn } = require("child_process");

const btPort = new SerialPort("/dev/rfcomm0");
btPort.on("beforeopen", () => {
  // the serial port opened by rfcomm does not close properly when the bluetooth connection terminates
  // helpfully, since `rfcomm listen hci0` terminates when the bluetooth connection closes,
  // we can kill the serial port when this process exits
  const logger = require("./logger.js")("rfcomm");
  // stdbuf to disable output buffering of rfcomm
  // https://unix.stackexchange.com/a/25378
  const rfcomm = spawn("sudo", "stdbuf -oL -eL rfcomm listen hci0".split(" "));

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
    btPort.close();
  });
});
btPort.open();

setInterval(() => {}, 10000);
