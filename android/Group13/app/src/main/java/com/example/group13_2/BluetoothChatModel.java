package com.example.group13_2;

import android.Manifest;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;
import androidx.annotation.Nullable;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentActivity;

public class BluetoothChatModel extends Fragment {
    private ListView chatView;
    private EditText outText;
    private Button sendButton;
    private Button connectButton;
    private String connectedDevice = null;
    private ArrayAdapter<String> chatArrayAdapter;
    private StringBuffer outMessageBuffer;
    private BluetoothAdapter bluetoothAdapter = null;
    private BluetoothService bluetoothService = null;
    private static final int requestSecureDeviceConnect = 1;
    private static final int requestInsecureDeviceConnect = 2;
    private static final int requestEnableBluetooth = 3;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
        bluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        FragmentActivity activity = getActivity();
        if (bluetoothAdapter == null) {
            Toast.makeText(activity, "Bluetooth is not available", Toast.LENGTH_LONG).show();
            activity.finish();
        }

        // check if we already have location permissions
        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {

            AlertDialog alertDialog = new AlertDialog.Builder(activity).create();
            alertDialog.setTitle("Permissions Request");
            alertDialog.setMessage("Please take note that Location Permissions are required in order to enable bluetooth scanning.");
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    });
            alertDialog.show();
            // request the permission
            ActivityCompat.requestPermissions(activity,
                    new String[]{Manifest.permission.ACCESS_FINE_LOCATION},
                    1234);
        }

    }


    @Override
    public void onStart() {
        super.onStart();
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, requestEnableBluetooth);
            // Otherwise, setup the chat session
        } else if (bluetoothService == null) {
            setupChat();
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (bluetoothService != null) {
            bluetoothService.stop();
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        if (bluetoothService != null) {
            // Only if the state is STATE_NONE, do we know that we haven't started already
            if (bluetoothService.getState() == BluetoothService.STATE_NONE) {
                // Start the Bluetooth chat services
                bluetoothService.start();
            }
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container,
                             @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.bluetooth_chat, container, false);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        chatView = view.findViewById(R.id.in);
        outText = view.findViewById(R.id.edit_text_out);
        sendButton = view.findViewById(R.id.button_send);
        connectButton = view.findViewById(R.id.button_connect);
        chatView.setVisibility(View.INVISIBLE);
        outText.setVisibility(View.INVISIBLE);
        sendButton.setVisibility(View.INVISIBLE);
        connectButton.setVisibility(View.INVISIBLE);
    }

    public void showChat(boolean show){
        if(show){
            chatView.setVisibility(View.VISIBLE);
            outText.setVisibility(View.VISIBLE);
            sendButton.setVisibility(View.VISIBLE);
            connectButton.setVisibility(View.VISIBLE);
        }else{
            chatView.setVisibility(View.INVISIBLE);
            outText.setVisibility(View.INVISIBLE);
            sendButton.setVisibility(View.INVISIBLE);
            connectButton.setVisibility(View.INVISIBLE);
        }
    }



    public void onRequestPermissionsResult(int requestCode,
                                           @Nullable String[] permissions, @Nullable  int[] grantResults) {
        switch (requestCode) {
            case 1234: { //location permissions
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                } else {
                    Toast.makeText(getActivity(), "Location permissions not granted", Toast.LENGTH_LONG).show();
                    getActivity().finish();
                }
            }
        }
    }

    private void setupChat() {
        chatArrayAdapter = new ArrayAdapter<>(getActivity(), R.layout.chat);
        chatView.setAdapter(chatArrayAdapter);
        //Listener for Chat messages - Popup Dialog
        chatView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position,
                                    long id) {
                // Send a chat using content of the edit text widget
                FragmentActivity activity = getActivity();
                String item = ((TextView) view).getText().toString();

                //Toast.makeText(activity, item, Toast.LENGTH_LONG).show();

                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(activity);

                // set title
                alertDialogBuilder.setTitle("Chat Messages");

                // set dialog chat
                alertDialogBuilder
                        .setMessage(item)
                        .setCancelable(false)
                        .setNegativeButton("Done",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                                dialog.cancel();
                            }
                        });

                // create alert dialog
                AlertDialog alertDialog = alertDialogBuilder.create();

                // show it
                alertDialog.show();

            }
        });

        // Initialize the compose field with a listener for the return key
        outText.setOnEditorActionListener(mWriteListener);

        // Initialize the send button with a listener that for click events
        sendButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                // Send a chat using content of the edit text widget
                View view = getView();
                if (null != view) {
                TextView textView = view.findViewById(R.id.edit_text_out);
                String message = textView.getText().toString();
                sendMessage(message);
                }
            }
        });

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent serverIntent = new Intent(getActivity(), BluetoothDeviceList.class);
                startActivityForResult(serverIntent, requestSecureDeviceConnect);
            }
        });


        // Initialize the BluetoothChatService to perform bluetooth connections
        bluetoothService = new BluetoothService(getActivity(), mHandler);

        // Initialize the buffer for outgoing messages
        outMessageBuffer = new StringBuffer("");
    }

    private void makeDiscoverable() {
        if (bluetoothAdapter.getScanMode() !=
                BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
            discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300);
            startActivity(discoverableIntent);
        }
    }

    protected boolean sendMessage(String message) {
        // Check that we're actually connected before trying anything
        if (bluetoothService.getState() != BluetoothService.STATE_CONNECTED) {
            Toast.makeText(getActivity(), R.string.not_connected, Toast.LENGTH_SHORT).show();
            return false;
        }



        // Check that there's actually something to send
        if (message.length() > 0) {
            // Get the chat bytes and tell the BluetoothChatService to write
            byte[] send = message.getBytes();
            bluetoothService.write(send);

            // Reset out string buffer to zero and clear the edit text field
            outMessageBuffer.setLength(0);
            outText.setText(outMessageBuffer);
        }
        return true;
    }

    private TextView.OnEditorActionListener mWriteListener
            = new TextView.OnEditorActionListener() {
        public boolean onEditorAction(TextView view, int actionId, KeyEvent event) {
            // If the action is a key-up event on the return key, send the chat
            if (actionId == EditorInfo.IME_NULL && event.getAction() == KeyEvent.ACTION_UP) {
                String message = view.getText().toString();
                sendMessage(message);
            }
            return true;
        }
    };

    private void setStatus(int resId) {
        FragmentActivity activity = getActivity();
        if (null == activity) {
            return;
        }
        final ActionBar actionBar = activity.getActionBar();
        if (null == actionBar) {
            return;
        }
        actionBar.setSubtitle(resId);
    }

    private void setStatus(CharSequence subTitle) {
        FragmentActivity activity = getActivity();
        if (null == activity) {
            return;
        }
        final ActionBar actionBar = activity.getActionBar();
        if (null == actionBar) {
            return;
        }
        actionBar.setSubtitle(subTitle);
    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            FragmentActivity activity = getActivity();
            switch (msg.what) {
                case MessageConstants.MESSAGE_STATE_CHANGE:
                    switch (msg.arg1) {
                        case BluetoothService.STATE_CONNECTED:
                            setStatus(getString(R.string.title_connected_to, connectedDevice));
                            chatArrayAdapter.clear();
                            break;
                        case BluetoothService.STATE_CONNECTING:
                            setStatus(R.string.title_connecting);
                            break;
                        case BluetoothService.STATE_LISTEN:
                        case BluetoothService.STATE_NONE:
                            setStatus(R.string.title_not_connected);
                            break;
                    }
                    break;
                case MessageConstants.MESSAGE_WRITE:
                    byte[] writeBuf = (byte[]) msg.obj;
                    // construct a string from the buffer
                    String writeMessage = new String(writeBuf);
                    chatArrayAdapter.add("Android: " + writeMessage);
                    break;
                case MessageConstants.MESSAGE_READ:
                    byte[] readBuf = (byte[]) msg.obj;
                    // construct a string from the valid bytes in the buffer
                    String readMessage = new String(readBuf, 0, msg.arg1);
                    String lines[] = readMessage.split("\\r?\\n");
                    for (String temp : lines) {
                        if(temp != null && !temp.isEmpty()){
                            chatArrayAdapter.add("Raspberry Pi: " + temp);
                            //READ MESSAGE AT MAIN ACTIVITY
                            ((MainActivity)getActivity()).incomingMessage(temp);
                        }
                    }
                    break;
                case MessageConstants.MESSAGE_DEVICE_NAME:
                    // save the connected device's name
                    connectedDevice = msg.getData().getString(Utils.DEVICE_NAME);
                    if (null != activity) {
                        Toast.makeText(activity, "Connected to "
                                + connectedDevice, Toast.LENGTH_SHORT).show();
                    }
                    break;
                case MessageConstants.MESSAGE_TOAST:
                    if (null != activity) {
                        Toast.makeText(activity, msg.getData().getString(Utils.TOAST),
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
            }
        }
    };

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case requestSecureDeviceConnect:
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, true);
                }
                break;
            case requestInsecureDeviceConnect:
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, false);
                }
                break;
            case requestEnableBluetooth:
                if (resultCode == Activity.RESULT_OK) {
                    setupChat();
                } else {
                    Toast.makeText(getActivity(), R.string.bt_not_enabled_leaving, Toast.LENGTH_SHORT).show();
                    getActivity().finish();
                }
        }
    }

    private void connectDevice(Intent data, boolean secure) {
        String address = data.getExtras().getString(BluetoothDeviceList.EXTRA_DEVICE_ADDRESS);
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        bluetoothService.connect(device, secure);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.secure_connect_scan: {
                Intent serverIntent = new Intent(getActivity(), BluetoothDeviceList.class);
                startActivityForResult(serverIntent, requestSecureDeviceConnect);
                return true;
            }
            case R.id.discoverable: {
                makeDiscoverable();
                return true;
            }
        }
        return false;
    }

    public boolean sendMsg(String message){
        return sendMessage(message);
    }
}
