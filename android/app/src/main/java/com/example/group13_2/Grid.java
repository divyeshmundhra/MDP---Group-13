package com.example.group13_2;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;
import android.view.GestureDetector;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Toast;
import java.lang.Math;

import java.util.ArrayList;

public class Grid extends View implements View.OnTouchListener {

    Paint boundary = new Paint();
    Paint initialZone = new Paint();
    Paint cellsExplored = new Paint();
    Paint robot = new Paint();
    Paint head = new Paint();
    Paint obstacle = new Paint();
    Paint waypoint = new Paint();
    Paint cellsUnexplored = new Paint();
    Paint idBlock = new Paint();
    Paint coordinates = new Paint();
    float space = 2;
    float paddingX = 2;
    float width = this.getWidth()-2*50;
    float height = this.getHeight()-50;
    float cellWidth = width/15f;
    float cellHeight = height/20f;
    int end_x;
    int end_y;
    String toastText;
    private static Toast mCurrentToast;
    private GestureDetector mDetector;

    public Grid(Context context) {
        super(context);
        idBlock.setColor(Color.WHITE);
        idBlock.setTypeface(Typeface.DEFAULT_BOLD);
        idBlock.setTextAlign(Paint.Align.CENTER);
        idBlock.setTextSize(80);
        boundary.setColor(Color.parseColor("#000000"));
        cellsExplored.setColor(Color.parseColor("#9DBEBB"));
        cellsUnexplored.setColor(Color.parseColor("#f4e9cd"));
        waypoint.setColor(Color.parseColor("#031926"));
        initialZone.setColor(Color.parseColor("#77aca2"));
        obstacle.setColor(Color.parseColor("#468189"));
        robot.setColor(Color.parseColor("#424f4f"));
        head.setColor((Color.parseColor("#77aca2")));
        coordinates.setColor(Color.parseColor("#031926"));
        coordinates.setTextAlign(Paint.Align.CENTER);
        coordinates.setTextSize(15);
    }

    public void setGesture(Context context){
        mDetector = new GestureDetector (context, new MyGestureListener());
    }

    @Override
    public boolean onTouchEvent(MotionEvent event){
        this.mDetector.onTouchEvent(event);
        return super.onTouchEvent(event);
    }

    protected void onDraw(Canvas canvas) {
        space = 2;
        height = this.getHeight()-space;
        width = this.getWidth()-2*space;

        if(height/20f<width/15f){
            width = height/20f*15f;
        }else{
            height= width/15f*20f;
        }

        idBlock.setTextSize(height/20f);
        canvas.drawRect(0,  0,  width,  height, cellsUnexplored);
        paintCoordinates(canvas);
        paintExploredTile(canvas);
        paintIdBlocks(canvas);
        paintWP(canvas);
        paintRobot(canvas);
    }

    public void rightSwipe() {
        MainActivity ma = (MainActivity) this.getContext();
        ma.rightGesture();
    }

    public void leftSwipe() {
        MainActivity ma = (MainActivity) this.getContext();
        ma.leftGesture();
    }

    public void topSwipe() {
        MainActivity ma = (MainActivity) this.getContext();
        ma.topGesture();
    }

    public void bottomSwipe() {
        MainActivity ma = (MainActivity) this.getContext();
        ma.bottomGesture();
    }


    private void paintWP(Canvas canvas) {
        GridPosition wp = GridWayPoint.getInstance().getGridPosition();
        if(wp!=null&&wp.getPosX()>=0&&wp.getPosX()<15&&wp.getPosY()<20&&wp.getPosY()>=0){
            float posX = (paddingX+wp.getPosX()*cellWidth);
            float posY = ((19-wp.getPosY())*cellHeight);
            canvas.drawRect(posX, posY, posX+cellWidth, posY+cellHeight, waypoint);
        }
    }



    private void paintCoordinates(Canvas canvas) {
        for(int x =0;x<15;x++){
            for(int y =0;y<20;y++){
                float posX = (paddingX+x*cellWidth);
                float posY = ((19-y)*cellHeight);
                canvas.drawText(x + "," + y, posX+(0.5f)*cellWidth, posY+cellHeight-10, coordinates);
            }
        }
    }

    private void paintIdBlocks(Canvas canvas) {
        ArrayList<GridIDblock> numberedBlocks = GridMap.getInstance().getNumberedBlocks();
        for(GridIDblock block:numberedBlocks) {
            float posX =  (block.getGridPosition().getPosX()) * cellWidth;
            float posY =  (19-block.getGridPosition().getPosY()) * cellHeight;
            float textposX =  (block.getGridPosition().getPosX()+0.5f) * cellWidth;
            float textposY =  (20-0.1f-block.getGridPosition().getPosY()) * cellHeight;
            canvas.drawRect(posX, posY, posX + cellWidth, posY + cellHeight, obstacle);
            canvas.drawText(block.getID(), textposX, textposY, idBlock);
        }
    }

    private void paintRobot(Canvas canvas) {
        RobotInstance r = RobotInstance.getInstance();
        if(r.getPosX()<1||r.getPosY()<1||r.getPosX()>13||r.getPosY()>18){
            return;
        }
        float cellWidth = width/15f;
        float cellHeight = height/20f;
        float xCenterPosition = r.getPosX()*cellWidth+ paddingX  +(cellWidth/2f);
        float yCenterPosition = (19f-r.getPosY())*cellWidth  +(cellHeight/2f);
        canvas.drawRect((paddingX+r.getPosX()*cellWidth)-cellWidth, ((19-r.getPosY())*cellHeight)-cellWidth, (paddingX+r.getPosX()*cellWidth)+(2*cellWidth), ((19-r.getPosY())*cellHeight)+(2*cellWidth), robot);
        float direction = r.getDirection();
        double radians = Math.toRadians(direction);
        float sensorCenterX = (float) (xCenterPosition+(cellWidth/1.5f*Math.sin(radians)));
        float sensorCenterY = (float) (yCenterPosition-(cellWidth/1.5f*Math.cos(radians)));
        canvas.drawCircle(sensorCenterX, sensorCenterY,cellWidth/3f, head);
    }

    @Override
    public boolean onTouch(View view, MotionEvent me) {
        try {
            Thread.sleep(50);
        } catch (InterruptedException e) {
            e.printStackTrace();
        }

        me.getSource();

        float X;
        float Y;

        float selectedX;
        float selectedY;
        float cellWidth = width/15f;
        float cellHeight = height/20f;

        int posX;
        int posY;

        MainActivity ma = (MainActivity) this.getContext();

        switch(me.getAction()) {
            case MotionEvent.ACTION_DOWN:
            case MotionEvent.ACTION_POINTER_UP:
            case MotionEvent.ACTION_MOVE: {
                X = me.getX();
                Y = me.getY();
                selectedX = X - paddingX;
                selectedY = Y;
                posX = (int) (selectedX / cellWidth);
                posY = 19 - (int) (selectedY / cellHeight);
                end_x = posX;
                end_y = posY;
                toastText = "tapped " + posX + ", " + posY;
                break;
            }
        }
        showToast(toastText);
        ma.tapOnGrid( end_x,  end_y);

        return true;
    }

    private void paintExploredTile(Canvas canvas) {
        int[][]explored = GridMap.getInstance().getExploredTiles();
        int[][]obstacles = GridMap.getInstance().getObstacles();
        for(int x =0;x<15;x++){
            for(int y =0;y<20;y++){
                if( explored[y][x] == 1){
                    float posX = (paddingX+x*cellWidth);
                    float posY = ((19-y)*cellHeight);
                    if(obstacles[y][x]==1){
                        canvas.drawRect(posX, posY, posX+cellWidth, posY+cellHeight, obstacle);
                    }else{
                        if((y==0&&x==0)||(y==0&&x==1)||(y==0&&x==2)||   (y==1&&x==0)||  (y==1&&x==1)||   (y==1&&x==2)|| (y==2&&x==0)||(y==2&&x==1)|| (y==2&&x==2)||
                                (y==19&&x==14)||   (y==19&&x==13)||   (y==19&&x==12)||    (y==18&&x==14)||   (y==18&&x==13)||     (y==18&&x==12)|| (y==17&&x==14)||    (y==17&&x==13)||  (y==17&&x==12)){
                            canvas.drawRect(posX, posY, posX+cellWidth, posY+cellHeight, initialZone);
                        }else{
                            canvas.drawText(x + "," + y, posX+(0.5f)*cellWidth, posY+cellHeight, coordinates);
                            canvas.drawRect(posX, posY, posX+cellWidth, posY+cellHeight, cellsExplored);
                        }
                    }
                }
            }
        }

        for(int i = 0;i<16;i++){
            canvas.drawLine(i*(width/15f)+ paddingX, 0, i*(width/15f)+ paddingX, height, boundary);
        }
        for(int i = 0;i<21;i++){
            canvas.drawLine(paddingX, i*(height/20f), paddingX +width,i*(height/20f), boundary);
        }
    }


    public void showToast(String text) {
        Context context = getContext();

        if (mCurrentToast != null) {
            mCurrentToast.cancel();
        }
        mCurrentToast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
        mCurrentToast.show();
    }



    @Override
    public boolean dispatchTouchEvent(MotionEvent e)
    {
        super.dispatchTouchEvent(e);
        if(mDetector==null){
            return true;
        }
        return mDetector.onTouchEvent(e);
    }

    class MyGestureListener extends GestureDetector.SimpleOnGestureListener {

        @Override
        public boolean onDown(MotionEvent event) {
            return true;
        }

        @Override
        public boolean onFling(MotionEvent e1, MotionEvent e2, float velocityX, float velocityY) {
            boolean result = false;
            try {
                float diffY = e2.getY() - e1.getY();
                float diffX = e2.getX() - e1.getX();
                if (Math.abs(diffX) > Math.abs(diffY)) {
                    if (Math.abs(diffX) > 100 && Math.abs(velocityX) > 100) {
                        if (diffX > 0) {
                            rightSwipe();
                        } else {
                            leftSwipe();
                        }
                        result = true;
                    }
                }
                else if (Math.abs(diffY) > 100 && Math.abs(velocityY) > 100) {
                    if (diffY > 0) {
                        bottomSwipe();
                    } else {
                        topSwipe();
                    }
                    result = true;
                }
            } catch (Exception exception) {
                exception.printStackTrace();
            }
            return result;
        }
    }
}
