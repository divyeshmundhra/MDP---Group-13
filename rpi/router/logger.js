const { createLogger, format, transports } = require("winston");
const { combine, timestamp, label, printf } = format;

const logger = createLogger({
  level: "info",
  format: format.simple(),
  format: combine(
    timestamp(),
    printf(({ level, message, timestamp, service }) => {
      return `${timestamp} ${
        typeof service !== "undefined" ? `[${service}] ` : ""
      }${level}: ${message}`;
    })
  ),
  transports: [
    new transports.File({ filename: "combined.log", level: "verbose" }),
    new transports.Console(),
  ],
});

module.exports = (service) => {
  return logger.child({ service });
};
