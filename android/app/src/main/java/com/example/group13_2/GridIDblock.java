package com.example.group13_2;

public class GridIDblock {
	private String id;
	private GridPosition gridPosition;

	public String getID() { return id; }

	public GridPosition getGridPosition() {
		return gridPosition;
	}

	public GridIDblock(String blockID, int posX, int posY){
		this.gridPosition = new GridPosition(posX, posY);
		this.id = blockID;
	}
}
