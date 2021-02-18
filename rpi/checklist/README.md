# Pairing

Devices have to be paired before they can connect. To pair:

Run `bluetoothctl`, then `discoverable on`. Select the raspberry pi (mdp13) from the Android device

# Connecting

Ensure rfcomm is listening ie `sudo rfcomm listen hci0`, then connect from the Serial app on the Android device

# Run

`npm install` (only on checkout)
`node index.js`
