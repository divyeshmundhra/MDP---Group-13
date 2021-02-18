const http = require("http");
const SerialPort = require("serialport");
const Readline = require("@serialport/parser-readline");
const { spawn } = require("child_process");

const HTTP_PORT = 8080;

function onEvent(label, display) {
  return (line) => {
    const logline = `[${label}]: ${line}`;

    console.log(logline);
    if (display) {
      displayData += logline;
    }
  };
}

let displayData = "";

const btPort = new SerialPort("/dev/rfcomm0", { baudRate: 115200 });
console.log("Opened bt port");

const mcuPort = new SerialPort("/dev/ttyACM0", { baudRate: 115200 });
console.log("Opened mcu port");

const btStream = btPort.pipe(new Readline());
btStream.on("data", (line) => {
  onEvent("BT RX", true)(line);
  // arduino will echo lines prefixed with e
  mcuPort.write(`e${line.trim()}\n`);
});

const mcuStream = mcuPort.pipe(new Readline());
mcuStream.on("data", onEvent("MCU RX", true));

http
  .createServer((req, res) => {
    if (req.url === "/data") {
      res.writeHead(200, { "Content-Type": "text/plain" });
      res.write(displayData);
      res.end();
    } else {
      res.writeHead(200, { "Content-Type": "text/html" });
      res.write(`
        <h1>Group 13</h1>
        <script>
          setInterval(() => {
            fetch("/data").then((res) => res.text()).then((data) => {
              document.querySelector("#display").innerText = data;
            })
          }, 250);
        </script>
        <div id="display"></div>
      `);

      res.end();
    }
  })
  .listen(HTTP_PORT, (err) => {
    if (err) {
      console.err("Failed to start http server:", err);
      return;
    }

    console.log(`Listening on ${HTTP_PORT}`);
  });
