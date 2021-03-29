package com.example.group13_2;

import androidx.appcompat.app.AppCompatActivity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.app.AlertDialog;
import android.widget.GridLayout;
import android.widget.ImageButton;
import android.widget.RadioButton;
import android.widget.Switch;
import android.widget.TextView;
import androidx.fragment.app.FragmentTransaction;
import android.hardware.SensorEvent;
import android.widget.Toast;
import android.content.Intent;
import android.speech.RecognizerIntent;
import java.lang.Math;
import java.util.ArrayList;

public class MainActivity extends AppCompatActivity implements SensorEventListener {
    private SensorManager sensorManager;
    private Sensor sensor;
    boolean tiltChecked;
    long lastUpdate = System.currentTimeMillis();

    BluetoothChatModel chatUtil;
    GridLayout base_layout;
    MenuItem show_chat_menu;
    MenuItem menu_set_config1;
    MenuItem menu_set_config2;
    MenuItem menu_display_string;
    MenuItem voice_option;
    Button btn_send_config1;
    Button btn_send_config2;
    Button btn_removeWp;
    Button btn_explore;
    Button btn_fastest_path;
    Button btn_terminate_exploration;
    Button btn_update;
    Button reset_button;
    ImageButton joystick_left;
    ImageButton joystick_right;
    ImageButton joystick_forward;
    RadioButton autoUpdateRadio;
    RadioButton manualUpdateRadio;
    TextView statusView;
    Switch set_wp;
    Switch set_robotPost;
    Switch gestureEnable;
    Switch tiltEnable;
    String status = "Idle";
    String incoming_robot_position="RobotPosition";
    String incoming_map_data = "MapData";
    String incoming_grid_layout = "Grid";
    String incoming_numbered_block = "NumberedBlock";
    String incoming_move_forward = "Moving Forward";
    String incoming_move_right = "Moving Right";
    String incoming_move_left = "Moving Left";
    String move_up = "Robot|Up";
    String move_down = "Robot|Down";
    String move_left = "Robot|Left";
    String move_right = "Robot|Right";

    public static final int VOICE_RECOGNITION_REQUEST_CODE = 1234;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btn_send_config1= findViewById(R.id.config1);
        btn_send_config2= findViewById(R.id.config2);
        joystick_left = findViewById(R.id.leftJoystick);
        joystick_right = findViewById(R.id.rightJoystick);
        joystick_forward = findViewById(R.id.forwardJoystick);
        base_layout = findViewById(R.id.base_layout);
        set_robotPost = findViewById(R.id.set_robotPos);
        set_wp = findViewById(R.id.set_waypoint);
        btn_removeWp = findViewById(R.id.remove_waypoint);
        btn_explore = findViewById(R.id.explore);
        btn_fastest_path = findViewById(R.id.fastest_path);
        btn_terminate_exploration = findViewById(R.id.terminate_exploration);
        reset_button = findViewById(R.id.reset);
        autoUpdateRadio = findViewById(R.id.radioAuto);
        manualUpdateRadio = findViewById(R.id.radioManual);
        btn_update = findViewById(R.id.update);
        statusView = findViewById(R.id.status);
        gestureEnable = findViewById(R.id.gestureSwitch);
        tiltEnable = findViewById(R.id.tiltSwitch);
        tiltChecked=false;

        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        sensor = sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        sensorManager.registerListener(this, sensor,SensorManager.SENSOR_DELAY_NORMAL );

        if (savedInstanceState == null) {
            chatUtil = new BluetoothChatModel();
            FragmentTransaction bluetoothTransaction = getSupportFragmentManager().beginTransaction();
            bluetoothTransaction.replace(R.id.fragment, chatUtil);
            bluetoothTransaction.commit();
        }

        btn_update.setEnabled(false);
        updateStatus(status);
        buttonFunction();
        onClickTiltSwitch();
        loadGrid();
    }

    public void onClickTiltSwitch() {
        tiltEnable.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    tiltChecked = true;
                    Toast.makeText(MainActivity.this, "Tilt Switch On!!", Toast.LENGTH_SHORT).show();
                } else {
                    tiltChecked = false;
                    Toast.makeText(MainActivity.this, "Tilt Switch Off!!", Toast.LENGTH_SHORT).show();
                }
            }
        });
    }

    public void startVoiceRecognitionActivity() {
        Intent intent = new Intent(RecognizerIntent.ACTION_RECOGNIZE_SPEECH);
        intent.putExtra(RecognizerIntent.EXTRA_LANGUAGE_MODEL,
                RecognizerIntent.LANGUAGE_MODEL_FREE_FORM);
        startActivityForResult(intent, VOICE_RECOGNITION_REQUEST_CODE);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == VOICE_RECOGNITION_REQUEST_CODE && resultCode == RESULT_OK) {

            ArrayList matches = data.getStringArrayListExtra(RecognizerIntent.EXTRA_RESULTS);

            if (matches.contains("forward")) {
                if(!RobotInstance.getInstance().invalidCoordinate()) {
                    RobotInstance.getInstance().moveForward(10);
                    outgoingMessage(move_up, 1);
                    loadGrid();
                }
            }
            else if (matches.contains("right")){
                RobotInstance.getInstance().rotateRight();
                outgoingMessage(move_right, 1);
                loadGrid();
            }
            else if (matches.contains("left")){
                RobotInstance.getInstance().rotateLeft();
                outgoingMessage(move_left, 1);
                loadGrid();
            }
            else if (matches.contains("start")){
                if (GridWayPoint.waypoint.getGridPosition() == null) {
                    updateStatus("Setting WayPoint");
                    set_robotPost.setChecked(false);
                    set_wp.setChecked(true);
                    Toast toast=Toast.makeText(getApplicationContext(),"Tap the Grid to set WayPoint",Toast.LENGTH_LONG);
                    toast.show();
                }else{
                    outgoingMessage("FP", 0);
                    updateStatus("Fastest Path");
                }
            }
            else if (matches.contains("explore")){
                outgoingMessage("EX", 0);
                updateStatus("Exploring");
            }
            else if (matches.contains("image")){
                outgoingMessage("IR", 0);
                updateStatus("Image Recognition");
            }
            else if (matches.contains("terminate")){
                outgoingMessage("TX", 0);
                updateStatus("Terminated Exploration");
            }
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        long curTime = System.currentTimeMillis();

        if ((curTime - lastUpdate) > 500){ // only reads data twice per second
            lastUpdate = curTime;
            float x = event.values[0];
            float y = event.values[1];
            if (tiltChecked) {
                if (Math.abs(x) > Math.abs(y)) {
                    if (x>2) {
                        updateStatus("Left Tilt");
                        if(!RobotInstance.getInstance().rotateToWest()){
                            if(!RobotInstance.getInstance().invalidCoordinate()) {
                                outgoingMessage(move_left, 1);
                                RobotInstance.getInstance().moveForward(10);
                            }
                        }
                        else{
                            Integer count = RobotInstance.getInstance().getCount();
                            sendMsgOnSwipe(count);
                            RobotInstance.getInstance().rotateRobotToWest();
                        }
                        loadGrid();
                    }
                    if (x < -2) {
                        updateStatus("Right Tilt");
                        if(!RobotInstance.getInstance().rotateToEast()){
                            if(!RobotInstance.getInstance().invalidCoordinate()) {
                                outgoingMessage(move_right, 1);
                                RobotInstance.getInstance().moveForward(10);
                            }
                        }
                        else{
                            Integer count = RobotInstance.getInstance().getCount();
                            sendMsgOnSwipe(count);
                            RobotInstance.getInstance().rotateRobotToEast();
                        }
                        loadGrid();
                    }
                } else {
                    if (y < -2) {
                        updateStatus("Up Tilt");
                        if(!RobotInstance.getInstance().rotateToNorth()){
                            if(!RobotInstance.getInstance().invalidCoordinate()) {
                                outgoingMessage(move_up, 1);
                                RobotInstance.getInstance().moveForward(10);
                            }
                        }
                        else{
                            Integer count = RobotInstance.getInstance().getCount();
                            sendMsgOnSwipe(count);
                            RobotInstance.getInstance().rotateRobotToNorth();
                        }
                        loadGrid();
                    }
                    if (y >2) {
                        updateStatus("Down Tilt");
                        if(!RobotInstance.getInstance().rotateToSouth()){
                            if(!RobotInstance.getInstance().invalidCoordinate()) {
                                outgoingMessage(move_down, 1);
                                RobotInstance.getInstance().moveForward(10);
                            }
                        }
                        else{
                            Integer count = RobotInstance.getInstance().getCount();
                            sendMsgOnSwipe(count);
                            RobotInstance.getInstance().rotateRobotToSouth();
                        }
                        loadGrid();
                    }
                }
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {
    }

    public void receivedMessage(String msg) {
        final RobotInstance r = RobotInstance.getInstance();
        if (msg.length() > 0) {
            chatUtil.showChat(true);
            String message[];
            if (msg.contains(":")) {
                message = msg.split(":");
            } else {
                message = msg.split("-");
            }

            if (message[0].equals(incoming_grid_layout)) { //get just P2
                GridMap.getInstance().setOnlyP2(message[1]);
                if (autoUpdateRadio.isChecked()) {
                    loadGrid();
                }
            } else if (message[0].equals(incoming_map_data)) {//get (P1,P2,position)
                String data[] = message[1].split(",");

                GridMap.getInstance().setArena(data[0], "", data[1]);

                r.setPosX(Float.parseFloat(data[2]));
                r.setPosY(Float.parseFloat(data[3]));
                r.setDirection(data[4]);
                if (autoUpdateRadio.isChecked()) {
                    loadGrid();
                }
            } else if (message[0].equals(incoming_numbered_block)) { //get image rec blocks
                String data[] = message[1].split(","); //x, y, id
                GridIDblock input = new GridIDblock(data[2], Integer.parseInt(data[0]), Integer.parseInt(data[1]));
                GridMap.getInstance().addIDBlocks(input);
                if (autoUpdateRadio.isChecked()) {
                    loadGrid();
                }
            } else if (message[0].equals(incoming_robot_position)) {
                String posAndDirect[] = message[1].split(",");
                r.setPosX(Float.parseFloat(posAndDirect[0]));
                r.setPosY(Float.parseFloat(posAndDirect[1]));
                r.setDirection(posAndDirect[2]);
                if (autoUpdateRadio.isChecked()) {
                    loadGrid();
                }
            }
            else if (message[0].equals("Msg")) {
                if (message[1].equals("F")) {
                    updateStatus(incoming_move_forward);
                }
                else if (message[1].equals("TR")) {
                    updateStatus(incoming_move_right);
                }
                else if (message[1].equals("TL")) {
                    updateStatus(incoming_move_left);
                }
                else if (message[1].equals("FP")) {
                    updateStatus("Fastest Path");
                }
                else if (message[1].equals("EX")) {
                    updateStatus("Exploring");
                }
                else if (message[1].equals("DONE")) {
                    updateStatus("Done!");
                }
                else {
                    updateStatus(message[1]);
                }
            } else if (message[0].trim().equals("Y")) {
                updateStatus("Moving");
            } else if (message[0].trim().equals("F")) {
                updateStatus("Done!");
            } else {
                updateStatus("Invalid Message");
            }
        }
    }

    public void updateStatus(String status){
        this.status = status;
        statusView.setText(status);
    }

    public boolean outgoingMessage(String message) {
        return chatUtil.sendMsg(message);
    }

    public boolean outgoingMessage(String message, int destination) {
        if(destination == 0){
            message = message+"\n";
        }else if(destination == 1){
            message = message+"\n";
        }
        return chatUtil.sendMsg(message);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        super.onPrepareOptionsMenu(menu);
        voice_option = menu.findItem(R.id.voice_btn);
        show_chat_menu = menu.findItem(R.id.action_show_bluetooth_chat);
        menu_set_config1 = menu.findItem(R.id.action_set_config_string1);
        menu_set_config2 = menu.findItem(R.id.action_set_config_string2);
        menu_display_string = menu.findItem(R.id.action_view_data_strings);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.action_show_bluetooth_chat) {
            boolean checked = item.isChecked();
            item.setChecked(!checked);
            if(chatUtil!=null){
                chatUtil.showChat(!checked);
            }
            return true;
        }
        if (id == R.id.action_set_config_string1) {
            configStr(1);
            return true;
        }
        if (id == R.id.action_set_config_string2) {
            configStr(2);
            return true;
        }
        if (id == R.id.action_view_data_strings) {
            displayDataStrings();
            return true;
        }
        if (id == R.id.voice_btn) {
            startVoiceRecognitionActivity();
        }
        return super.onOptionsItemSelected(item);
    }

    public static String binaryStringToHexadecimalString(String s) {

        String obstacles = "";
        String binaryStr = s;
        for(int i = 0; i < binaryStr.length(); i+=4){
            int decimal = Integer.parseInt(binaryStr.substring(i,i+4),2);
            String hexStr = Integer.toString(decimal,16);
            obstacles += hexStr;
        }
        return obstacles;
    }


    public void topGesture() {
        if(!RobotInstance.getInstance().rotateToNorth()){
            if(!RobotInstance.getInstance().invalidCoordinate()) {
                outgoingMessage(move_up, 1);
                RobotInstance.getInstance().moveForward(10);
            }
        }
        else{
            Integer count = RobotInstance.getInstance().getCount();
            sendMsgOnSwipe(count);
            RobotInstance.getInstance().rotateRobotToNorth();
        }
        loadGrid();
    }

    public void leftGesture() {
        if(!RobotInstance.getInstance().rotateToWest()){
            if(!RobotInstance.getInstance().invalidCoordinate()) {
                outgoingMessage(move_left, 1);
                RobotInstance.getInstance().moveForward(10);
            }
        }
        else{
            Integer count = RobotInstance.getInstance().getCount();
            sendMsgOnSwipe(count);
            RobotInstance.getInstance().rotateRobotToWest();
        }

        loadGrid();
    }

    public void rightGesture() {
        if(!RobotInstance.getInstance().rotateToEast()){
            if(!RobotInstance.getInstance().invalidCoordinate()) {
                outgoingMessage(move_right, 1);
                RobotInstance.getInstance().moveForward(10);
            }
        }
        else{
            Integer count = RobotInstance.getInstance().getCount();
            sendMsgOnSwipe(count);
            RobotInstance.getInstance().rotateRobotToEast();
        }
        loadGrid();
    }

    public void bottomGesture() {
        if(!RobotInstance.getInstance().rotateToSouth()){
            if(!RobotInstance.getInstance().invalidCoordinate()) {
                outgoingMessage(move_down, 1);
                RobotInstance.getInstance().moveForward(10);
            }
        }
        else{
            Integer count = RobotInstance.getInstance().getCount();
            sendMsgOnSwipe(count);
            RobotInstance.getInstance().rotateRobotToSouth();
        }
        loadGrid();
    }

    public void sendMsgOnSwipe(Integer count){

        if(count==1){
            outgoingMessage("D", 1);
        }
        if(count==2){
            outgoingMessage("D", 1);
            outgoingMessage("D", 1);
        }
        if(count==-1){
            outgoingMessage("A", 1);
        }
    }

    private void configStr(final int index){
        final EditText txtField = new EditText(this);
        SharedPreferences preferences = getSharedPreferences(String.valueOf(R.string.app_name), MODE_PRIVATE);
        String retrievedText = preferences.getString("string"+index, null);
        if (retrievedText != null) {
            txtField.setText(retrievedText);
        }

        new AlertDialog.Builder(this)
                .setTitle("Configure String "+index)
                .setView(txtField)
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        String input = txtField.getText().toString();
                        SharedPreferences.Editor editor = getSharedPreferences(("Group 13"), MODE_PRIVATE).edit();
                        editor.putString("string"+index, input);
                        editor.apply();
                    }
                })
                .setNegativeButton("Close", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                    }
                })
                .show();
    }

    private void loadGrid(){
        Grid mCustomDrawableView = new Grid(this);
        if(gestureEnable.isChecked()){
            mCustomDrawableView.setGesture(this);
        }else{
            mCustomDrawableView.setOnTouchListener(mCustomDrawableView);
        }
        base_layout.removeAllViews();
        base_layout.addView(mCustomDrawableView);
    }

    public void tapOnGrid(final int posX, final int posY) {
        if(set_robotPost.isChecked()){
            RobotInstance r = RobotInstance.getInstance();
            r.setPosX(posX);
            r.setPosY(posY);
            r.setDirection("NORTH");
            outgoingMessage("START:"+posX+","+posY, 0);
            set_robotPost.setChecked(false);
        }
        if(set_wp.isChecked()){
            GridPosition p = new GridPosition(posX,posY);
            GridWayPoint.getInstance().setGridPosition(p);
            outgoingMessage("WP:"+posX+","+posY, 0);
            set_wp.setChecked(false);
        }
        loadGrid();
    }

    private void buttonFunction(){
        btn_send_config1.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                SharedPreferences prefs = getSharedPreferences(String.valueOf(R.string.app_name), MODE_PRIVATE);
                String retrievedText = prefs.getString("string1", null);
                updateStatus("Sending Config 1");
                if (retrievedText != null) {
                    outgoingMessage(retrievedText);
                }
            }
        });
        btn_send_config2.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                SharedPreferences prefs = getSharedPreferences(String.valueOf(R.string.app_name), MODE_PRIVATE);
                String retrievedText = prefs.getString("string2", null);
                updateStatus("Sending Config 2");
                if (retrievedText != null) {
                    outgoingMessage(retrievedText);
                }
            }
        });
        joystick_forward.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                //check if robot is out of bounds
                if(!RobotInstance.getInstance().invalidCoordinate()) {
                    RobotInstance.getInstance().moveForward(10);
                    outgoingMessage(move_up, 1);
                    loadGrid();
                }
            }
        });
        joystick_left.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                RobotInstance.getInstance().rotateLeft();
                outgoingMessage(move_left, 1);
                loadGrid();
            }
        });
        joystick_right.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                RobotInstance.getInstance().rotateRight();
                outgoingMessage(move_right, 1);
                loadGrid();
            }
        });
        btn_removeWp.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                GridWayPoint.getInstance().setGridPosition(null);
                loadGrid();
            }
        });
        btn_fastest_path.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (GridWayPoint.waypoint.getGridPosition() == null) {
                    updateStatus("Setting WayPoint");
                    set_robotPost.setChecked(false);
                    set_wp.setChecked(true);
                    Toast toast=Toast.makeText(getApplicationContext(),"Tap the Grid to set WayPoint",Toast.LENGTH_LONG);
                    toast.show();
                }else{
                    outgoingMessage("FP", 0);
                    updateStatus("Fastest Path");
                }
            }
        });
        btn_terminate_exploration.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                outgoingMessage("TX", 0);
                updateStatus("Terminated Exploration");
            }
        });
        btn_explore.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                outgoingMessage("EX", 0);
                updateStatus("Exploring");
            }
        });
        reset_button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                updateStatus("Reset Map");
                final RobotInstance r = RobotInstance.getInstance();
                GridMap.getInstance().setArena(
                        "0000000000000000000000000000000000000000000000000000000000000000000000000000",
                        "",
                        "0000000000000000000000000000000000000000000000000000000000000000000000000000");
                r.setPosX(Float.parseFloat("1"));
                r.setPosY(Float.parseFloat("1"));
                r.setDirection("NORTH");
                GridMap.getInstance().clearNumberedBlocks();
                loadGrid();

            }
        });
        manualUpdateRadio.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                btn_update.setEnabled(true);
            }
        });
        autoUpdateRadio.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                btn_update.setEnabled(false);
            }
        });
        btn_update.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                loadGrid();
            }
        });

    }

    private void displayDataStrings(){
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Map Descriptor and Image Recognition String");
        final View customLayout = getLayoutInflater().inflate(R.layout.mdf_string, null);

        builder.setView(customLayout);
        builder.setPositiveButton("CLOSE", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        });

        EditText text_mdf1 = customLayout.findViewById(R.id.textbox_mdf1);
        String explored = binaryStringToHexadecimalString(GridMap.getInstance().getExploredCells());
        text_mdf1.setText(explored);

        EditText text_mdf2 = customLayout.findViewById(R.id.textbox_mdf2);
        String obstacles = binaryStringToHexadecimalString(GridMap.getInstance().getExploredObstacles());
        text_mdf2.setText(obstacles);

        EditText text_imgreg = customLayout.findViewById(R.id.textbox_imgreg);
        String imgreg = "{";
        ArrayList<GridIDblock> numberedBlocks = GridMap.getInstance().getNumberedBlocks();
        for(GridIDblock blk : numberedBlocks){
            imgreg += String.format("(%s, %d, %d)", blk.getID(), blk.getGridPosition().getPosX(), blk.getGridPosition().getPosY());
            imgreg += ", ";
        }
        if(imgreg.length()>2)imgreg = imgreg.substring(0, imgreg.length()-2);
        imgreg += "}";
        text_imgreg.setText(imgreg);
        AlertDialog dialog = builder.create();
        dialog.show();
    }

    @Override
    protected void onStop() {
        super.onStop();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
