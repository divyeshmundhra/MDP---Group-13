package com.example.group13_2;

import java.math.BigInteger;
import java.util.ArrayList;

public class GridMap {
	public static GridMap map=null;
	public static GridMap getInstance(){
		if(map==null){
			map= new GridMap();
		}
		return map;
	}
	private static int exploredTiles [][] = new int[20][15];
	private static int obstacles [][] = new int[20][15];
	private ArrayList<GridIDblock> idBlocks = new ArrayList<GridIDblock>();
	public  int[][] getObstacles() {
		return obstacles;
	}
	public int[][] getExploredTiles() {
		return exploredTiles;
	}
	public ArrayList<GridIDblock> getNumberedBlocks() { return idBlocks; }

	public void addIDBlocks(GridIDblock block) {
		int x = Math.min(block.getGridPosition().getCoordinateX(),14);
		int y = Math.min(block.getGridPosition().getCoordinateY(),19);
		x = Math.max(0,x);
		y = Math.max(0,y);
		block = new GridIDblock(block.getID(),x,y);

		for (GridIDblock ib:idBlocks) {
			if(ib.getGridPosition().equals(block.getGridPosition()) || ib.getID().equals(block.getID())){
				idBlocks.remove(ib);
				break;
			}
		}
		idBlocks.add(block);
	}

	public static String hexadecimalStringToBinaryString(String s) {
		if(s!=null){
			s="F"+s;
			s = new BigInteger(s, 16).toString(2);
			s = s.substring(4);
			return s ;
		}
		return "";
	}

	public void clearNumberedBlocks() {
		idBlocks.clear();
	}

	public String getExploredCells(){
		String pad="11";
		int array[][]= GridMap.getInstance().getExploredTiles();
		for(int y =0;y<20;y++){
			for(int x =0;x<15;x++){
				pad=pad+array[y][x];
			}
		}
		pad=pad+"11";
		return pad;
	}

	public String getExploredObstacles(){
		String string="";
		int arrayExplored[][]= GridMap.getInstance().getExploredTiles();
		int arrayObstacle[][]= GridMap.getInstance().getObstacles();
		for(int y =0;y<20;y++){
			for(int x =0;x<15;x++){
				if(arrayExplored[y][x]==1){
					string=string+arrayObstacle[y][x];
				}
			}
		}
		if(string.length()%8!=0){
			for(int i =0;i<string.length()%8;i++){
				string=string+"0";
			}
		}
		return string;
	}

	public void setArena(String exploredCells, String obstacleCells, String exploredObstacleCells) {

		String stringObstacle = hexadecimalStringToBinaryString(obstacleCells);
		if(stringObstacle.length()>0){
			stringObstacle =stringObstacle.substring(2, stringObstacle.length()-2);
		}

		String cellsExplored = hexadecimalStringToBinaryString(exploredCells);
		cellsExplored = cellsExplored.substring(2, cellsExplored.length()-2);
		  
		String exploredObstacleBinary=null;
		if(exploredObstacleCells!=null){
			exploredObstacleBinary = hexadecimalStringToBinaryString(exploredObstacleCells);
		}

		int obstacleIndex=0;
		for(int i =0;i<cellsExplored.length();i++){
			char charExplored = cellsExplored.charAt(i);
			char charObstacle = '0';
			if(stringObstacle.length()>0){
				charObstacle = stringObstacle.charAt(i);
			}
			int y = (i-(i%15))/15;
			int x = i%15;
			exploredTiles[y][x]=0;
			obstacles[y][x]=0;
			if(charObstacle=='1'){
				obstacles[y][x]=1;
			}
			if(charExplored=='1'){
				exploredTiles[y][x]=1;
				if(exploredObstacleBinary!=null){
					char exploredObstacleBit = exploredObstacleBinary.charAt(obstacleIndex);

					  if(exploredObstacleBit=='1'){
						  obstacles[y][x]=1;
					  }
				}
				obstacleIndex++;
			}
			  
			if((x==0&&y==0)||(x==1&&y==0)||(x==2&&y==0)||
				  (x==0&&y==1)||(x==1&&y==1)||(x==2&&y==1)||
				  (x==0&&y==2)||(x==1&&y==2)||(x==2&&y==2)){
				  obstacles[y][x]=0;
			}

			if((x==14&&y==19)||(x==13&&y==19)||(x==12&&y==19)||
				  (x==14&&y==18)||(x==13&&y==18)||(x==12&&y==18)||
				  (x==14&&y==17)||(x==13&&y==17)||(x==12&&y==17)){
				  obstacles[y][x]=0;
			}
		}
	}

	public void setOnlyP2( String exploredObstacleHex){

		String obstacleHex="";
		String exploredTileHex="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
		String cellsExplored = hexadecimalStringToBinaryString(exploredTileHex);
		cellsExplored = cellsExplored.substring(2, cellsExplored.length()-2);

		String obstacleBinary = hexadecimalStringToBinaryString(obstacleHex);
		if(obstacleBinary.length()>0){
			obstacleBinary =obstacleBinary.substring(2, obstacleBinary.length()-2);
		}

		String exploredObstacleBinary=null;
		if(exploredObstacleHex!=null){
			exploredObstacleBinary = hexadecimalStringToBinaryString(exploredObstacleHex);
		}

		int obstacleIndex=0;
		for(int i =0;i<cellsExplored.length();i++){
			char exploreBit = cellsExplored.charAt(i);
			char obstacleBit = '0';
			if(obstacleBinary.length()>0){
				obstacleBit = obstacleBinary.charAt(i);
			}
			int y = (i-(i%15))/15;
			int x = i%15;
			exploredTiles[y][x]=0;
			obstacles[y][x]=0;
			if(obstacleBit=='1'){
				obstacles[y][x]=1;
			}
			if(exploreBit=='1'){
				exploredTiles[y][x]=1;
				if(exploredObstacleBinary!=null){
					char exploredObstacleBit = exploredObstacleBinary.charAt(obstacleIndex);

					if(exploredObstacleBit=='1'){
						obstacles[y][x]=1;
					}
				}
				obstacleIndex++;
			}

			if((x==0&&y==0)||(x==1&&y==0)||(x==2&&y==0)||
					(x==0&&y==1)||(x==1&&y==1)||(x==2&&y==1)||
					(x==0&&y==2)||(x==1&&y==2)||(x==2&&y==2)){
				obstacles[y][x]=0;
			}

			if((x==14&&y==19)||(x==13&&y==19)||(x==12&&y==19)||
					(x==14&&y==18)||(x==13&&y==18)||(x==12&&y==18)||
					(x==14&&y==17)||(x==13&&y==17)||(x==12&&y==17)){
				obstacles[y][x]=0;
			}
		}
	}


}
