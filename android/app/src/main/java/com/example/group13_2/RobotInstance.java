package com.example.group13_2;

public class RobotInstance {
    public static RobotInstance robot = null;

    public static RobotInstance getInstance() {
        if (robot == null) {
            robot = new RobotInstance();
        }
        return robot;
    }


    public RobotInstance() {
    }

    private float posX = -1f;
    private float posY = -1f;
    private float direction = 0;

    public int count;

    public float getCoordinateX() {
        return posX;
    }

    public void setCoordinateX(float posX) {
        this.posX = posX;
    }

    public float getCoordinateY() {
        return posY;
    }

    public void setCoordinateY(float posY) {
        this.posY = posY;
    }




    public void rotate(float degree) {

        direction = (direction + degree) % 360;
    }

    public void rotateRobotToNorth() {
        if (direction != 0) {
            float degree = (int) degreeToRotateToDirection(direction, 0);
            rotate(degree);
        }
    }

    public void rotateRobotToSouth() {
        if (direction != 180 && direction != -180) {
            float degree = (int) degreeToRotateToDirection(direction, 180);
            rotate(degree);
        }
    }

    public void rotateRobotToEast() {
        if (direction != 90 && direction != -270) {
            float degree = (int) degreeToRotateToDirection(direction, 90);
            rotate(degree);
        }
    }

    public void rotateRobotToWest() {
        if (direction != 270 && direction != -90) {
            float degree = (int) degreeToRotateToDirection(direction, 270);
            rotate(degree);
        }
    }

    public void rotateRight() {
        rotate(90);
    }

    public void rotateLeft() {
        rotate(-90);
    }


    public boolean invalidCoordinate() {
        int posX = (int) getCoordinateX();
        int posY = (int) getCoordinateY();
        int direction = (int) getRobotDirection();

        if ((posX >= 13 && posY <= 1 && (direction == 180 || direction == 90 || direction == -180 || direction == -270)))
            return true;
        else if ((posX <= 1 && posY >= 18 && (direction == 0 || direction == 270 || direction == -90)))
            return true;
        else if ((posX <= 1 && posY <= 1 && (direction == 270 || direction == 180 || direction == -90 || direction == -180)))
            return true;
        else if (posY <= 1 && (direction == 180 || direction == -180))
            return true;
        else if (posX <= 1 && (direction == 270 || direction == -90))
            return true;
        else if ((posX >= 13 && posY >= 18 && (direction == 0 || direction == 90 || direction == -270)))
            return true;
        else if (posX >= 13 && (direction == 90 || direction == -270))
            return true;
        else if (posY >= 18 && direction == 0)
            return true;
        else
            return false;
    }

    public void robotMove(int distance) {
        double radians = Math.toRadians(direction);
        float moveX = ((distance / 10f) * (float) Math.sin(radians));
        float moveY = ((distance / 10f) * (float) Math.cos(radians));
        if (getCoordinateX() + moveX > 13) {
            setCoordinateX(13);
        } else {
            if (getCoordinateX() + moveX < 1) {
                setCoordinateX(1);
            } else {
                setCoordinateX(getCoordinateX() + moveX);
            }
        }

        if (getCoordinateY() + moveY > 18) {
            setCoordinateY(18);
        } else {
            if (getCoordinateY() + moveY < 1) {
                setCoordinateY(1);
            } else {
                setCoordinateY(getCoordinateY() + moveY);
            }
        }
    }

    public float getRobotDirection() {
        return direction;
    }

    public void setRobotDirection(String direction) {
        switch (direction) {
            case "NORTH":
                this.direction = 0;
                break;
            case "SOUTH":
                this.direction = 180;
                break;
            case "EAST":
                this.direction = 90;
                break;
            case "WEST":
                this.direction = 270;
                break;
        }
    }

    public int getCount() {
        return count;
    }

    private float degreeToRotateToDirection(float currentDirection, float inDirection) {
        float difference = inDirection - currentDirection;
        if (Math.abs(Math.round(difference)) == 180) {
            return 180;
        }
        if (difference < 180) {
            if (Math.abs(difference) > 180) {
                return difference + 360;
            } else {
                return difference;
            }
        } else {
            return (-(360 - difference));
        }
    }

    public boolean rotateToNorth() {
        if (direction == 0) {
            return false;
        }
        float degree = (int) degreeToRotateToDirection(direction, 0);
        int degree_int = (int) degree;
        if (degree_int == 90) {
            count = 1;
        }
        if (degree_int == 180) {

            count = 2;
        }
        if (degree_int == -90) {
            count = -1;
        }
        rotate(degree);

        return true;
    }

    public boolean rotateToSouth() {
        if (direction == 180 || direction == -180) {
            return false;
        }
        float degree = (int) degreeToRotateToDirection(direction, 180);

        int degree_int = (int) degree;
        if (degree_int == 90) {
            count = 1;
        }
        if (degree_int == 180) {

            count = 2;
        }
        if (degree_int == -90) {
            count = -1;
        }
        rotate(degree);
        return true;
    }

    public boolean rotateToEast() {
        if (direction == 90 || direction == -270) {
            return false;
        }
        float degree = (int) degreeToRotateToDirection(direction, 90);
        int degree_int = (int) degree;
        if (degree_int == 90) {
            count = 1;
        }
        if (degree_int == 180) {

            count = 2;
        }
        if (degree_int == -90) {
            count = -1;
        }

        rotate(degree);
        return true;
    }


    public boolean rotateToWest() {
        if (direction == 270 || direction == -90) {
            return false;
        }
        float degree = (int) degreeToRotateToDirection(direction, 270);
        rotate(degree);
        int degree_int = (int) degree;
        if (degree_int == 90) {
            count = 1;
        }
        if (degree_int == 180) {

            count = 2;
        }
        if (degree_int == -90) {
            count = -1;
        }
        return true;
    }


}