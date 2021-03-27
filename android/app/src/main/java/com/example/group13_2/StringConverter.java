package com.example.group13_2;

import java.math.BigInteger;

public class StringConverter {
	public static String hexadecimalStringToBinaryString(String s) {
		if(s!=null){
			s="F"+s;
			s = new BigInteger(s, 16).toString(2);
			s = s.substring(4);
		  return s ;
		}
		return "";
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
}
