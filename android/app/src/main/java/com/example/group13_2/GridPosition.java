package com.example.group13_2;

public class GridPosition {
	int posX;
	int posY;
	
	public GridPosition(int posX, int posY){
		this.posX=posX;
		this.posY=posY;
	}

	public boolean equals(GridPosition pos) {
		if(posX==pos.getCoordinateX() && posY==pos.getCoordinateY()){
			return true;
		}
		return false;
	}

	public int getCoordinateX() {
		return posX;
	}
	public int getCoordinateY() {
		return posY;
	}

}
