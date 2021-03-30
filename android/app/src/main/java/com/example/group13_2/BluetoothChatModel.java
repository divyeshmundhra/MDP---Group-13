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

        if (ContextCompat.checkSelfPermission(activity,
                Manifest.permission.ACCESS_FINE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {

            AlertDialog alertDialog = new AlertDialog.Builder(activity).create();
            alertDialog.setTitle("Request for Permissions");
            alertDialog.setMessage("Please enable location services for bluetooth.");
            alertDialog.setButton(AlertDialog.BUTTON_NEUTRAL, "OK",
                    new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            dialog.dismiss();
                        }
                    });
            alertDialog.show();
            ActivityCompat.requestPermissions(activity,
                    new String[]{Manifest.permission.ACCESS_FINE_LOCATION},
                    1234);
        }

    }

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            FragmentActivity activity = getActivity();
            switch (msg.what) {
                case 1:
                    switch (msg.arg1) {
                        case BluetoothService.BLUETOOTH_CONNECTED:
                            setStatus(getString(R.string.title_connected_to, connectedDevice));
                            chatArrayAdapter.clear();
                            break;
                        case BluetoothService.BLUETOOTH_CONNECTING:
                            setStatus("Connecting...");
                            break;
                        case BluetoothService.BLUETOOTH_LISTEN:
                        case BluetoothService.IDLE:
                            setStatus("not connected");
                            break;
                    }
                    break;
                case 3:
                    byte[] writeBuf = (byte[]) msg.obj;
                    String writeMessage = new String(writeBuf);
                    chatArrayAdapter.add("Android: " + writeMessage);
                    break;
                case 2:
                    byte[] readBuf = (byte[]) msg.obj;
                    String readMessage = new String(readBuf, 0, msg.arg1);
                    String lines[] = readMessage.split("\\r?\\n");
                    for (String temp : lines) {
                        if(temp != null && !temp.isEmpty()){
                            chatArrayAdapter.add("Raspberry Pi: " + temp);
                            ((MainActivity)getActivity()).receivedMessage(temp);
                        }
                    }
                    break;
                case 4:
                    connectedDevice = msg.getData().getString("bluetooth_device_name");
                    if (null != activity) {
                        Toast.makeText(activity, "Connected to "
                                + connectedDevice, Toast.LENGTH_SHORT).show();
                    }
                    break;
                case 5:
                    if (null != activity) {
                        Toast.makeText(activity, msg.getData().getString("toast"),
                                Toast.LENGTH_SHORT).show();
                    }
                    break;
            }
        }
    };


    @Override
    public void onStart() {
        super.onStart();
        if (!bluetoothAdapter.isEnabled()) {
            Intent enableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableIntent, 3);
        } else if (bluetoothService == null) {
            startChat();
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
            if (bluetoothService.getState() == BluetoothService.IDLE) {
                bluetoothService.start();
            }
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, @Nullable ViewGroup container, @Nullable Bundle savedInstanceState) {
        return inflater.inflate(R.layout.bluetooth_chat, container, false);
    }

    private void makeDiscoverable() {
        if (bluetoothAdapter.getScanMode() !=
                BluetoothAdapter.SCAN_MODE_CONNECTABLE_DISCOVERABLE) {
            Intent discoverableIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
            discoverableIntent.putExtra(BluetoothAdapter.EXTRA_DISCOVERABLE_DURATION, 300);
            startActivity(discoverableIntent);
        }
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
            case 1234: {
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                } else {
                    Toast.makeText(getActivity(), "Permission for location not given.", Toast.LENGTH_LONG).show();
                    getActivity().finish();
                }
            }
        }
    }

    private void startChat() {
        chatArrayAdapter = new ArrayAdapter<>(getActivity(), R.layout.chat);
        chatView.setAdapter(chatArrayAdapter);
        chatView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position,
                                    long id) {
                FragmentActivity activity = getActivity();
                String item = ((TextView) view).getText().toString();
                AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(activity);
                alertDialogBuilder.setTitle("Chat Messages");
                alertDialogBuilder
                        .setMessage(item)
                        .setCancelable(false)
                        .setNegativeButton("Done",new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog,int id) {
                                dialog.cancel();
                            }
                        });
                AlertDialog alertDialog = alertDialogBuilder.create();
                alertDialog.show();

            }
        });

        outText.setOnEditorActionListener(mWriteListener);
        sendButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
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
                startActivityForResult(serverIntent, 1);
            }
        });

        bluetoothService = new BluetoothService(getActivity(), mHandler);
        outMessageBuffer = new StringBuffer();
    }



    protected boolean sendMessage(String message) {
        if (bluetoothService.getState() != BluetoothService.BLUETOOTH_CONNECTED) {
            Toast.makeText(getActivity(), "You are not connected to a device", Toast.LENGTH_SHORT).show();
            return false;
        }


        if (message.length() > 0) {
            byte[] send = message.getBytes();
            bluetoothService.write(send);
            outMessageBuffer.setLength(0);
            outText.setText(outMessageBuffer);
        }
        return true;
    }

    private TextView.OnEditorActionListener mWriteListener
            = new TextView.OnEditorActionListener() {
        public boolean onEditorAction(TextView view, int actionId, KeyEvent event) {
            if (actionId == EditorInfo.IME_NULL && event.getAction() == KeyEvent.ACTION_UP) {
                String message = view.getText().toString();
                sendMessage(message);
            }
            return true;
        }
    };

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode) {
            case 1:
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, true);
                }
                break;
            case 2:
                if (resultCode == Activity.RESULT_OK) {
                    connectDevice(data, false);
                }
                break;
            case 3:
                if (resultCode == Activity.RESULT_OK) {
                    startChat();
                } else {
                    Toast.makeText(getActivity(), "Bluetooth was not enabled. Exit Chat.", Toast.LENGTH_SHORT).show();
                    getActivity().finish();
                }
        }
    }

    private void connectDevice(Intent data, boolean secure) {
        String address = data.getExtras().getString("device_address");
        BluetoothDevice device = bluetoothAdapter.getRemoteDevice(address);
        bluetoothService.connect(device, secure);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.secure_connect_scan: {
                Intent serverIntent = new Intent(getActivity(), BluetoothDeviceList.class);
                startActivityForResult(serverIntent, 1);
                return true;
            }
            case R.id.discoverable: {
                makeDiscoverable();
                return true;
            }
        }
        return false;
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

    public boolean sendMsg(String message){
        return sendMessage(message);
    }
}
