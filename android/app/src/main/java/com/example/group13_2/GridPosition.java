package com.example.group13_2;

public class GridPosition {
	int posX;
	int posY;
	
	public GridPosition(int posX, int posY){
		this.posX=posX;
		this.posY=posY;
	}

	public boolean equals(GridPosition arg0) {
		if(posX==arg0.getPosX() && posY==arg0.getPosY()){
			return true;
		}
		return false;
	}

	public int getPosX() {
		return posX;
	}
	public int getPosY() {
		return posY;
	}

}
