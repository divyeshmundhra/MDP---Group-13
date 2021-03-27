package com.example.group13_2;


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
	private static String obstacleString = "";
	private ArrayList<GridIDblock> numberedBlocks = new ArrayList<GridIDblock>();
	public  int[][] getObstacles() {
		return obstacles;
	}
	public int[][] getExploredTiles() {
		return exploredTiles;
	}
	public ArrayList<GridIDblock> getNumberedBlocks() { return numberedBlocks; }

	public void addNumberedBlocks(GridIDblock block) {

		int x = Math.min(block.getGridPosition().getPosX(),14);
		int y = Math.min(block.getGridPosition().getPosY(),19);
		x = Math.max(0,x);
		y = Math.max(0,y);
		block = new GridIDblock(block.getID(),x,y);

		for (GridIDblock nb:numberedBlocks) {
			if(nb.getGridPosition().equals(block.getGridPosition()) || nb.getID().equals(block.getID())){
				numberedBlocks.remove(nb);
				break;
			}
		}


		numberedBlocks.add(block);
	}

	public void clearNumberedBlocks() {
		numberedBlocks.clear();
	}

	//in hexdecimal, from map descriptor file to arrays
	public void setMap(String exploredTileHex, String obstacleHex, String exploredObstacleHex) {

		obstacleString = exploredObstacleHex;
		String exploredTileBinary = StringConverter.hexadecimalStringToBinaryString(exploredTileHex);
		exploredTileBinary = exploredTileBinary.substring(2, exploredTileBinary.length()-2);
		  
		String obstacleBinary = StringConverter.hexadecimalStringToBinaryString(obstacleHex);
		if(obstacleBinary.length()>0){
			obstacleBinary =obstacleBinary.substring(2, obstacleBinary.length()-2);
		}
		  
		String exploredObstacleBinary=null;
		if(exploredObstacleHex!=null){
			exploredObstacleBinary = StringConverter.hexadecimalStringToBinaryString(exploredObstacleHex);
		}
		  
		int obstacleIndex=0;
		for(int i =0;i<exploredTileBinary.length();i++){
			char exploreBit = exploredTileBinary.charAt(i);
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

	public void setOnlyP2( String exploredObstacleHex){
		obstacleString = exploredObstacleHex;
		String obstacleHex="";
		String exploredTileHex="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
		String exploredTileBinary = StringConverter.hexadecimalStringToBinaryString(exploredTileHex);
		exploredTileBinary = exploredTileBinary.substring(2, exploredTileBinary.length()-2);

		String obstacleBinary = StringConverter.hexadecimalStringToBinaryString(obstacleHex);
		if(obstacleBinary.length()>0){
			obstacleBinary =obstacleBinary.substring(2, obstacleBinary.length()-2);
		}

		String exploredObstacleBinary=null;
		if(exploredObstacleHex!=null){
			exploredObstacleBinary = StringConverter.hexadecimalStringToBinaryString(exploredObstacleHex);
		}

		int obstacleIndex=0;
		for(int i =0;i<exploredTileBinary.length();i++){
			char exploreBit = exploredTileBinary.charAt(i);
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

	public void setMapJson(String obstacleHex){
		String exploredTileHex="FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";

		String exploredTileBinary = StringConverter.hexadecimalStringToBinaryString(exploredTileHex);
		exploredTileBinary =exploredTileBinary.substring(2, exploredTileBinary.length()-2);

		String obstacleBinary = StringConverter.hexadecimalStringToBinaryString(obstacleHex);

		for(int i =0;i<exploredTileBinary.length();i++){
			char exploreBit = exploredTileBinary.charAt(i);
			int y = (i-(i%15))/15;
			int x = i%15;
			exploredTiles[y][x]=0;
			if(exploreBit=='1'){
				exploredTiles[y][x]=1;
			}
		}

		for(int i =0;i<obstacleBinary.length();i++){
			char obstacleBit = '0';
			if(obstacleBinary.length()>0){
				obstacleBit = obstacleBinary.charAt(i);
			}
			int y = (i-(i%15))/15;
			int x = i%15;
			obstacles[19-y][x]=0;
			if(obstacleBit=='1'){
				obstacles[19-y][x]=1;
			}
		}
	}

	public String getBinaryExplored(){
		String binaryExplored="11";
		int exploredTile[][]= GridMap.getInstance().getExploredTiles();
		for(int y =0;y<20;y++){
			for(int x =0;x<15;x++){
				binaryExplored=binaryExplored+exploredTile[y][x];
			}
		}
		binaryExplored=binaryExplored+"11";
		return binaryExplored;
	}
	public String getBinaryExploredObstacle(){
		//String binaryExplored="11";
		String binaryExploredObstacle="";
		int exploredTile[][]= GridMap.getInstance().getExploredTiles();
		int obstacles[][]= GridMap.getInstance().getObstacles();
		for(int y =0;y<20;y++){
			for(int x =0;x<15;x++){
				if(exploredTile[y][x]==1){
					binaryExploredObstacle=binaryExploredObstacle+obstacles[y][x];
				}
			}
		}
		//binaryExplored=binaryExplored+"11";
		if(binaryExploredObstacle.length()%8!=0){
			for(int i =0;i<binaryExploredObstacle.length()%8;i++){
				binaryExploredObstacle=binaryExploredObstacle+"0";
			}
		}
		return binaryExploredObstacle;
	}
}
