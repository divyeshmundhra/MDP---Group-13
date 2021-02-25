const Comms = require("./Comms.js");
const Controller = require("./Controller.js");
const Robot = require("./Robot.js");
const config = require("./config.js");

const comms = new Comms(config.zmq.broadcastAddress, config.zmq.updateAddress);
const controller = new Controller(config.serialPorts.btPort);
const robot = new Robot(config.serialPorts.arduinoPort);

const logger = require("./logger.js")("index");

controller.on("data", (data) => {
  if (data === "START") {
    comms.send({ type: "start" });
  }
});

comms.on("data", ({ type, data }) => {
  if (type === "move") {
    if ("turn" in data) {
      const DIRECTION_MAP = {
        left: "L",
        right: "R",
      };

      const { turn, direction } = data;

      if (typeof turn !== "number") {
        logger.warn(`Expected turn to be number but got ${typeof turn}`);
        return;
      }

      if (!Object.keys(DIRECTION_MAP).includes(direction)) {
        logger.warn(`Unexpected direction ${direction}`);
        return;
      }

      if (turn % 90 != 0) {
        logger.warn(`Expected turn angle to be multiple of 90 but got ${turn}`);
        return;
      }

      // divide by 45 because the robot expects 45 degree multiples
      robot.send(`${DIRECTION_MAP[direction]}${Math.floor(turn / 45)}`);
    } else if ("advance" in data) {
      const DIRECTION_MAP = {
        forward: "F",
        backward: "B",
      };

      const { advance, direction } = data;

      if (typeof advance !== "number" || advance <= 0) {
        logger.warn(
          `Expected advance to be positive number but got ${typeof advance}`
        );
        return;
      }

      if (!Object.keys(DIRECTION_MAP).includes(direction)) {
        logger.warn(`Unexpected direction ${direction}`);
        return;
      }

      robot.send(`${DIRECTION_MAP[direction]}${advance}`);
    }
  }
});

robot.on("move", (direction) => {
  comms.send({ type: "move_done", data: { direction } });
});

robot.on("sensors", (update) => {
  comms.send({ type: "sensor", data: update });
});
