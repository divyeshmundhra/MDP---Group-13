package com.example.group13_2;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;


public class BluetoothService {
    private final BluetoothAdapter bluetoothAdapter;
    private final Handler mHandler;
    private AcceptThread mSecureAcceptThread;
    private AcceptThread mInsecureAcceptThread;
    private ConnectThread mConnectThread;
    private ConnectedThread mConnectedThread;
    private int mState;
    private int mNewState;
    public static final int IDLE = 0;
    public static final int BLUETOOTH_LISTEN = 1;
    public static final int BLUETOOTH_CONNECTING = 2;
    public static final int BLUETOOTH_CONNECTED = 3;

    public BluetoothService(Context context, Handler handler) {
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        mState = IDLE;
        mNewState = mState;
        mHandler = handler;
    }

    public synchronized void start() {
        if (mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        if (mSecureAcceptThread == null) {
            mSecureAcceptThread = new AcceptThread(true);
            mSecureAcceptThread.start();
        }
        if (mInsecureAcceptThread == null) {
            mInsecureAcceptThread = new AcceptThread(false);
            mInsecureAcceptThread.start();
        }

        updateUserInterfaceTitle();
    }

    public synchronized void stop() {

        if (mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        if (mSecureAcceptThread != null) {
            mSecureAcceptThread.cancel();
            mSecureAcceptThread = null;
        }

        if (mInsecureAcceptThread != null) {
            mInsecureAcceptThread.cancel();
            mInsecureAcceptThread = null;
        }
        mState = IDLE;
        updateUserInterfaceTitle();
    }

    public synchronized int getState() {
        return mState;
    }

    public void write(byte[] out) {

        ConnectedThread r;
        synchronized (this) {
            if (mState != BLUETOOTH_CONNECTED) return;
            r = mConnectedThread;
        }
        r.write(out);
    }

    public synchronized void connect(BluetoothDevice device, boolean secure) {

        if (mState == BLUETOOTH_CONNECTING) {
            if (mConnectThread != null) {
                mConnectThread.cancel();
                mConnectThread = null;
            }
        }

        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        mConnectThread = new ConnectThread(device, secure);
        mConnectThread.start();
        updateUserInterfaceTitle();
    }

    public synchronized void connected(BluetoothSocket socket, BluetoothDevice
            device, final String socketType) {

        if (mConnectThread != null) {
            mConnectThread.cancel();
            mConnectThread = null;
        }

        if (mConnectedThread != null) {
            mConnectedThread.cancel();
            mConnectedThread = null;
        }

        if (mSecureAcceptThread != null) {
            mSecureAcceptThread.cancel();
            mSecureAcceptThread = null;
        }
        if (mInsecureAcceptThread != null) {
            mInsecureAcceptThread.cancel();
            mInsecureAcceptThread = null;
        }

        mConnectedThread = new ConnectedThread(socket, socketType);
        mConnectedThread.start();
        Message msg = mHandler.obtainMessage(4);
        Bundle bundle = new Bundle();
        bundle.putString("device_name", device.getName());
        msg.setData(bundle);
        mHandler.sendMessage(msg);
        updateUserInterfaceTitle();
    }

    private synchronized void updateUserInterfaceTitle() {
        mState = getState();
        mNewState = mState;
        mHandler.obtainMessage(1, mNewState, -1).sendToTarget();
    }

    private class AcceptThread extends Thread {
        private final BluetoothServerSocket mmServerSocket;
        private String mSocketType;

        public AcceptThread(boolean secure) {
            BluetoothServerSocket tmp = null;
            mSocketType = secure ? "Secure" : "Insecure";
            try {
                if (secure) {
                    tmp = bluetoothAdapter.listenUsingRfcommWithServiceRecord("BluetoothChatSecure", UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"));
                } else {
                    tmp = bluetoothAdapter.listenUsingInsecureRfcommWithServiceRecord("BluetoothChatInsecure", UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"));
                }
            } catch (IOException e) {
            }
            mmServerSocket = tmp;
            mState = BLUETOOTH_LISTEN;
        }

        public void run() {
            setName("AcceptThread" + mSocketType);

            BluetoothSocket socket = null;
            while (mState != BLUETOOTH_CONNECTED) {
                try {
                    socket = mmServerSocket.accept();
                } catch (IOException e) {
                    break;
                }

                if (socket != null) {
                    synchronized (BluetoothService.this) {
                        switch (mState) {
                            case BLUETOOTH_LISTEN:
                            case BLUETOOTH_CONNECTING:
                                connected(socket, socket.getRemoteDevice(),
                                        mSocketType);
                                break;
                            case IDLE:
                            case BLUETOOTH_CONNECTED:
                                try {
                                    socket.close();
                                } catch (IOException e) {
                                }
                                break;
                        }
                    }
                }
            }

        }

        public void cancel() {
            try {
                mmServerSocket.close();
            } catch (IOException e) {
            }
        }
    }

    private class ConnectThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final BluetoothDevice mmDevice;
        private String mSocketType;

        public ConnectThread(BluetoothDevice device, boolean secure) {
            mmDevice = device;
            BluetoothSocket tmp = null;
            mSocketType = secure ? "Secure" : "Insecure";
            try {
                if (secure) {
                    tmp = device.createRfcommSocketToServiceRecord(
                            UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"));
                } else {
                    tmp = device.createInsecureRfcommSocketToServiceRecord(
                            UUID.fromString("00001101-0000-1000-8000-00805f9b34fb"));
                }
            } catch (IOException e) {
            }
            mmSocket = tmp;
            mState = BLUETOOTH_CONNECTING;
        }

        public void connectionFailed() {
            Message msg = mHandler.obtainMessage(5);
            Bundle bundle = new Bundle();
            bundle.putString("toast", "Unable to connect device");
            msg.setData(bundle);
            mHandler.sendMessage(msg);

            mState = IDLE;
            updateUserInterfaceTitle();
            BluetoothService.this.start();
        }

        public void run() {
            setName("ConnectThread" + mSocketType);
            bluetoothAdapter.cancelDiscovery();
            try {
                mmSocket.connect();
            } catch (IOException e) {
                try {
                    mmSocket.close();
                } catch (IOException e2) {
                }
                connectionFailed();
                return;
            }

            synchronized (BluetoothService.this) {
                mConnectThread = null;
            }

            connected(mmSocket, mmDevice, mSocketType);
        }

        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
            }
        }
    }

    private class ConnectedThread extends Thread {
        private final BluetoothSocket mmSocket;
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket, String socketType) {
            mmSocket = socket;
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
            }

            mmInStream = tmpIn;
            mmOutStream = tmpOut;
            mState = BLUETOOTH_CONNECTED;
        }

        public void run() {
            byte[] buffer = new byte[1024];
            int bytes;
            while (mState == BLUETOOTH_CONNECTED) {
                try {
                    bytes = mmInStream.read(buffer);
                    mHandler.obtainMessage(2, bytes, -1, buffer)
                            .sendToTarget();
                } catch (IOException e) {
                    connectionLost();
                    break;
                }
            }
        }

        private void connectionLost() {
            Message msg = mHandler.obtainMessage(5);
            Bundle bundle = new Bundle();
            bundle.putString("toast", "Device connection was lost");
            msg.setData(bundle);
            mHandler.sendMessage(msg);

            mState = IDLE;
            updateUserInterfaceTitle();
            BluetoothService.this.start();
        }

        public void write(byte[] buffer) {
            try {
                mmOutStream.write(buffer);
                mHandler.obtainMessage(3, -1, -1, buffer)
                        .sendToTarget();
            } catch (IOException e) {
            }
        }

        public void cancel() {
            try {
                mmSocket.close();
            } catch (IOException e) {
            }
        }
    }
}
