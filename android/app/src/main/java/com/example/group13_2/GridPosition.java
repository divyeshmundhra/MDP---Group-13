package com.example.group13_2;

public class GridPosition {
	int coordinateX;
	int coordinateY;
	
	public GridPosition(int coordinateX, int coordinateY){
		this.coordinateX=coordinateX;
		this.coordinateY=coordinateY;
	}

	public boolean equals(GridPosition pos) {
		if(coordinateX==pos.getCoordinateX() && coordinateY==pos.getCoordinateY()){
			return true;
		}
		return false;
	}

	public int getCoordinateX() { return coordinateX; }
	public int getCoordinateY() {
		return coordinateY;
	}

}
