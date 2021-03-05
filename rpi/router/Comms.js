const zmq = require("zeromq");
const { EventEmitter } = require("events");

const logger = require("./logger.js")("comms");

class Comms extends EventEmitter {
  constructor(txAddress, rxAddress, configAddress) {
    super();

    this._socketInit(txAddress, rxAddress, configAddress);
  }

  async _socketInit(txAddress, rxAddress, configAddress) {
    // broadcasts updates from controller/robot
    this.tx = new zmq.Publisher();
    // gets updates from algo and cv
    this.rx = new zmq.Pull();
    // for config/pingpong
    this.config = new zmq.Reply();

    await this.tx.bind(txAddress);
    await this.rx.bind(rxAddress);
    await this.config.bind(configAddress);

    logger.info(`init with tx: ${txAddress}, rx: ${rxAddress}`);
    this._receiverTask();
    this._reqReceiverTask();
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

  setReqHandler(func) {
    this.reqHandler = func;
  }

  async _reqReceiverTask() {
    for await (const [msg] of this.config) {
      if (msg.toString() === "ping") {
        await this.config.send("pong");
        continue;
      }

      if (typeof this.reqHandler !== "function") {
        logger.warn("request received but no req handler defined!");
        await this.config.send("req handler not defined!");
        continue;
      }

      await this.config.send(this.reqHandler(JSON.parse(msg)));
    }
  }
}

module.exports = Comms;
