Various utility scripts:

- `set_new_fp.py`: Used to write a new arena (P2) string into the RPi, this P2 string will be automatically sent to the algorithm when a fastest path run is started
- `send_echo.py`: Request the RPi router to re-broadcast a message. For example, to simulate the android tablet sending a start for image recognition or exploration:

  ```
  python3 send_echo.py '{"type":"init", "data":{"task" :"EX", "arena":{"P1": "0","P2": "0"}}}'
  python3 send_echo.py '{"type":"init", "data":{"task" :"IR", "arena":{"P1": "0","P2": "0"}}}'
  ```
- `viz.py`: Used to graph raw binary data from the Arduino (and to act as a serial terminal for debugging)
