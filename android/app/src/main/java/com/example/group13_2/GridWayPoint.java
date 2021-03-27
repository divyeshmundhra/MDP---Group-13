package com.example.group13_2;

public class GridWayPoint {
	public static GridWayPoint waypoint=null;
	private GridPosition gridPosition =null;
	public static GridWayPoint getInstance(){
		if(waypoint==null){
			waypoint= new GridWayPoint();
		}
		return waypoint;
	}

	public GridPosition getGridPosition() {
		return gridPosition;
	}
	public void setGridPosition(GridPosition gridPosition) {
		this.gridPosition = gridPosition;
	}

}
