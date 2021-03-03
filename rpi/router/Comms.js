const zmq = require("zeromq");
const { EventEmitter } = require("events");

const logger = require("./logger.js")("comms");

class Comms extends EventEmitter {
  constructor(txAddress, rxAddress) {
    super();

    this._socketInit(txAddress, rxAddress);
  }

  async _socketInit(txAddress, rxAddress) {
    // broadcasts updates from controller/robot
    this.tx = new zmq.Publisher();
    // gets updates from algo and cv
    this.rx = new zmq.Pull();

    await this.tx.bind(txAddress);
    await this.rx.bind(rxAddress);

    logger.info(`init with tx: ${txAddress}, rx: ${rxAddress}`);
    this._receiverTask();
  }

  async _receiverTask() {
    for await (const [msg] of this.rx) {
      try {
        logger.verbose(`RX: ${msg}`);
        this.emit("data", JSON.parse(msg));
      } catch (err) {
        logger.warn(err);
      }
    }
  }

  async send(data) {
    logger.verbose(`TX: ${JSON.stringify(data)}`);
    await this.tx.send(JSON.stringify(data));
  }
}

module.exports = Comms;
