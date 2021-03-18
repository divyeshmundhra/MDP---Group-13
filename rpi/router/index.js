const Comms = require("./Comms.js");
const Controller = require("./Controller.js");
const Robot = require("./Robot.js");
const config = require("./config.js");

const store = require("data-store")({
  path: require("path").join(process.cwd(), config.dataStoreFileName),
});

const comms = new Comms(
  config.zmq.broadcastAddress,
  config.zmq.updateAddress,
  config.zmq.configAddress
);
const controller = new Controller(config.serialPorts.btPort);
const robot = new Robot(config.serialPorts.arduinoPort);

const logger = require("./logger.js")("index");

const state = {
  mode: false,
};

controller.on("data", (data) => {
  if (data === "FP") {
    state.mode = "FP";
    logger.info(`Mode set to ${state.mode}`);
    comms.send({
      type: "init",
      data: {
        task: "FP",
        arena: {
          P1: "0",
          P2: store.get("fp_p2"),
        },
      },
    });
    comms.send({ type: "start" });
  } else if (data === "EX") {
    state.mode = "EX";
    logger.info(`Mode set to ${state.mode}`);
    comms.send({
      type: "init",
      data: {
        task: "EX",
        arena: {
          P1: "0",
          P2: "0",
        },
      },
    });
    comms.send({ type: "start" });
  } else if (data === "IR") {
    state.mode = "EX";
    logger.info(`Mode set to ${state.mode}`);
    comms.send({
      type: "init",
      data: {
        task: "IR",
        arena: {
          P1: "0",
          P2: "0",
        },
      },
    });
    robot.send("D0");
    robot.once("sensors", () => {
      comms.send({ type: "start" });
    });

    robot.send("Qa");
  } else if (data.startsWith("WP:")) {
    const match = data.match(/WP:(\d+),(\d+)/);
    if (!match) {
      logger.error(`Malformed WP: ${data}`);
      return;
    }

    const x = parseInt(match[1], 10);
    const y = parseInt(match[2], 10);

    comms.send({ type: "waypoint", data: { x, y } });
    controller.send(`Grid:${store.get("fp_p2")}`);
  } else if (data === "TX") {
    comms.send({ type: "terminate" });
  }
});

comms.on("data", ({ type, data }) => {
  if (type === "echo") {
    logger.info(`echoing ${JSON.stringify(data)}`);
    comms.send(data);
  } else if (type === "turn") {
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
  } else if (type === "advance") {
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
  } else if (type === "logsensors") {
    robot.send("QA");
  } else if (type === "status") {
    const { x, y, orientation } = data["robot_info"];
    const { P1, P2 } = data["internal_arena"];
    if (state.mode === "FP") {
      controller.send(`RobotPosition:${x},${y},${orientation}`);
    } else if (state.mode === "EX") {
      controller.send(`MapData:${P1},${P2},${x},${y},${orientation}`);
    } else {
      logger.warn("Received status while mode not set");
    }

    comms.send({ type: "robotinfo", data: data["robot_info"] });
  } else if (type === "detection") {
    controller.send(`NumberedBlock:${data["x"]},${data["y"]},${data["id"]}`);
  }
});

comms.setReqHandler(({ type, data }) => {
  if (type === "setfparena") {
    const { P2 } = data;

    if (typeof P2 === "undefined") {
      return "Expected P2 string but not defined";
    }

    let reply = "ok";

    if (P2.length !== 76) {
      reply = `Expected P2 string of length 76 but got ${P2.length} instead. Still saving string`;
    }

    if (P2.match(/[^0-9a-fA-F]/, P2)) {
      reply = "Invalid hex character in string. Still saving string";
    }

    store.set("fp_p2", P2);
    logger.info(`Set new fastest path arena: ${P2}`);

    return reply;
  }
});

robot.on("move", (direction) => {
  comms.send({ type: "move_done", data: { direction } });
});

robot.on("sensors", (update) => {
  comms.send({ type: "sensor", data: update });
});
