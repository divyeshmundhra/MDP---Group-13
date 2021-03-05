module.exports = {
  logFileName: "combined.log",
  serialPorts: {
    btPort: "/dev/rfcomm0",
    arduinoPort: "/dev/ttyACM0",
    connectionRetry: 100,
  },
  zmq: {
    broadcastAddress: "tcp://0.0.0.0:3000",
    updateAddress: "tcp://0.0.0.0:3001",
    configAddress: "tcp://0.0.0.0:3002",
  },
};
