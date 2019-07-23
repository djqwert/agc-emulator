/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#include <iostream>
#include <string.h>
#include <sstream>

#include "DSKYLogic.h"

using namespace std;

DSKYLogic::DSKYLogic(){
	blinker = true;
	blinkerCounter = 0;
	strobeCounter = 0;
	
	for(int i=0; i<18; i++){
		lamps[i] = false;
	}
	
	for(int i=0; i<31; i++){
		digits[i] = '0';
	}
	digits[6] = ' ';
	digits[12] = ' ';
	digits[18] = ' ';
}

char DSKYLogic::bitmapToDigit(uint8_t input){
	input = input & 0b00011111;
	if(input == 0b00000000)
		return ' ';
	else if(input == 0b00010101)
		return '0';
	else if(input == 0b00000011)
		return '1';
	else if(input == 0b00011001)
		return '2';
	else if(input == 0b00011011)
		return '3';
	else if(input == 0b00001111)
		return '4';
	else if(input == 0b00011110)
		return '5';
	else if(input == 0b00011100)
		return '6';
	else if(input == 0b00010011)
		return '7';
	else if(input == 0b00011101)
		return '8';
	else if(input == 0b00011111)
		return '9';
	else
		return 'e';//Error (Does not exist on real AGC)
}

void DSKYLogic::clearStrobes(){
	if(lamps[14] == true){
		strobeCounter++;
		if(strobeCounter > STROBE_PERIOD){
			lamps[14] = false;
			strobeCounter = 0;
		}
	}
}

string DSKYLogic::getStatus(){
	char subbuff[7];
	
	memcpy(subbuff, &digits[0], 2);
	subbuff[2] = '\0';
	string reg0(subbuff);
	
	if(!verbBlinker || blinker){
		memcpy(subbuff, &digits[2], 2);
		subbuff[2] = '\0';
	}
	else{
		subbuff[0] = ' ';
		subbuff[1] = ' ';
		subbuff[2] = '\0';
	}
	string reg1(subbuff);
	
	if(!nounBlinker || blinker){
		memcpy(subbuff, &digits[4], 2);
		subbuff[2] = '\0';
	}
	else{
		subbuff[0] = ' ';
		subbuff[1] = ' ';
		subbuff[2] = '\0';
	}
	string reg2(subbuff);
	
	memcpy(subbuff, &digits[6], 6);
	subbuff[6] = '\0';
	string reg3(subbuff);
	
	memcpy(subbuff, &digits[12], 6);
	subbuff[6] = '\0';
	string reg4(subbuff);
	
	memcpy(subbuff, &digits[18], 6);
	subbuff[6] = '\0';
	string reg5(subbuff);
	
	
	stringstream buffer;
	
	buffer << "{\x22success\x22:true,\x22lamps\x22:["
	<< "{\x22id\x22:\x22lamp_1\x22,\x22value\x22:" << lamps[0] << "},"
	<< "{\x22id\x22:\x22lamp_2\x22,\x22value\x22:" << lamps[1] << "},"
	<< "{\x22id\x22:\x22lamp_3\x22,\x22value\x22:" << lamps[2] << "},"
	<< "{\x22id\x22:\x22lamp_4\x22,\x22value\x22:" << (lamps[3] & blinker) << "},"
	<< "{\x22id\x22:\x22lamp_5\x22,\x22value\x22:" << (lamps[4] & blinker) << "},"
	<< "{\x22id\x22:\x22lamp_6\x22,\x22value\x22:" << lamps[5] << "},"
	<< "{\x22id\x22:\x22lamp_7\x22,\x22value\x22:" << lamps[6] << "},"
	<< "{\x22id\x22:\x22lamp_8\x22,\x22value\x22:" << lamps[7] << "},"
	<< "{\x22id\x22:\x22lamp_9\x22,\x22value\x22:" << lamps[8] << "},"
	<< "{\x22id\x22:\x22lamp_10\x22,\x22value\x22:" << lamps[9] << "},"
	<< "{\x22id\x22:\x22lamp_11\x22,\x22value\x22:" << lamps[10] << "},"
	<< "{\x22id\x22:\x22lamp_12\x22,\x22value\x22:" << lamps[11] << "},"
	<< "{\x22id\x22:\x22lamp_13\x22,\x22value\x22:" << lamps[12] << "},"
	<< "{\x22id\x22:\x22lamp_14\x22,\x22value\x22:" << lamps[13] << "},"
	<< "{\x22id\x22:\"disp_lamp_1\x22,\x22value\x22:" << lamps[14] << "},"
	<< "{\x22id\x22:\"disp_lamp_2\x22,\x22value\x22:" << lamps[15] << "},"
	<< "{\x22id\x22:\"disp_lamp_3\x22,\x22value\x22:" << lamps[16] << "},"
	<< "{\x22id\x22:\"disp_lamp_4\x22,\x22value\x22:" << lamps[17] << "}"
	<< "],\"digits\x22:["
	<< "{\x22id\x22:\"digits_1\x22,\x22value\x22:\"" << reg0 << "\"},"
	<< "{\x22id\x22:\"digits_2\x22,\x22value\x22:\"" << reg1 << "\"},"
	<< "{\x22id\x22:\"digits_3\x22,\x22value\x22:\"" << reg2 << "\"},"
	<< "{\x22id\x22:\"digits_4\x22,\x22value\x22:\"" << reg3 << "\"},"
	<< "{\x22id\x22:\"digits_5\x22,\x22value\x22:\"" << reg4 << "\"},"
	<< "{\x22id\x22:\"digits_6\x22,\x22value\x22:\"" << reg5 << "\"}"
	<< "]}";
	
	string responseBody = buffer.str();
	return responseBody;
}

void DSKYLogic::toggleBlinker(){
	blinkerCounter++;
	if(blinkerCounter > BLINKER_PERIOD){
		blinker = !blinker;
		blinkerCounter = 0;
	}
}

void DSKYLogic::write8(uint16_t word){
	word = word >> 1;
	int signIndex;
	int digitIndex_0;
	int digitIndex_1;
	
	uint16_t position = (word >> 11) & 0b0000000000001111;
	uint16_t signBit = (word >> 10) & 0b0000000000000001;
	uint16_t firstDigit = (word >> 5) & 0b0000000000011111;
	uint16_t secondDigit = word & 0b0000000000011111;
	
	if(position == 11){
		signIndex = 30;
		digitIndex_0 = 0;
		digitIndex_1 = 1;
	}
	else if(position == 10){
		signIndex = 30;
		digitIndex_0 = 2;
		digitIndex_1 = 3;
	}
	else if(position == 9){
		signIndex = 30;
		digitIndex_0 = 4;
		digitIndex_1 = 5;
	}
	else if(position == 8){
		signIndex = 30;
		digitIndex_0 = 30;
		digitIndex_1 = 7;
	}
	else if(position == 7){
		signIndex = 24;
		digitIndex_0 = 8;
		digitIndex_1 = 9;
	}
	else if(position == 6){
		signIndex = 25;
		digitIndex_0 = 10;
		digitIndex_1 = 11;
	}
	else if(position == 5){
		signIndex = 26;
		digitIndex_0 = 13;
		digitIndex_1 = 14;
	}
	else if(position == 4){
		signIndex = 27;
		digitIndex_0 = 15;
		digitIndex_1 = 16;
	}
	else if(position == 3){
		signIndex = 30;
		digitIndex_0 = 17;
		digitIndex_1 = 19;
	}
	else if(position == 2){
		signIndex = 28;
		digitIndex_0 = 20;
		digitIndex_1 = 21;
	}
	else if(position == 1){
		signIndex = 29;
		digitIndex_0 = 22;
		digitIndex_1 = 23;
	}
	else if(position == 12){//Special case: lamps
		if((word & 0b0000000000000001) == 0)//bit 1 -> PRIO DISP
			;//lamps[]	????????
		
		if((word & 0b0000000000000010) == 0)//bit 2 -> NO DAP
			;//lamps[]	????????
		
		if((word & 0b0000000000000100) == 0)//bit 3 -> VEL
			lamps[13] = false;
		else
			lamps[13] = true;
		
		if((word & 0b0000000000001000) == 0)//bit 4 -> NO ATT
			lamps[1] = false;
		else
			lamps[1] = true;
		
		if((word & 0b0000000000010000) == 0)//bit 5 -> ALT
			lamps[12] = false;
		else
			lamps[12] = true;
		
		if((word & 0b0000000000100000) == 0)//bit 6 -> GIMBALL LOCK
			lamps[8] = false;
		else
			lamps[8] = true;
		
		if((word & 0b0000000010000000) == 0)//bit 8 -> TRACKER
			lamps[11] = false;
		else
			lamps[11] = true;
		
		if((word & 0b0000000100000000) == 0)//bit 9 -> PROG
			lamps[9] = false;
		else
			lamps[9] = true;
		
		//From there on bits are fictitious
		
		if((word & 0b0000001000000000) == 0)//bit 10 -> STBY
			lamps[2] = false;
		else
			lamps[2] = true;
		
		if((word & 0b0000010000000000) == 0)//bit 11 -> RESTART
			lamps[10] = false;
		else
			lamps[10] = true;
			
		return;
	}
	else{
		cout << "DSKY output error" << endl;
		return;
	}
	
	//Decoding the output value
	digits[signIndex] = (signBit == 1)?'1':'0';
	digits[digitIndex_0] = bitmapToDigit(firstDigit);
	digits[digitIndex_1] = bitmapToDigit(secondDigit);
	
	//Sign decoding
	if(digits[24] == '1' && digits[25] == '0'){
		digits[6] = '+';
	}
	else if(digits[24] == '0' && digits[25] == '1'){
		digits[6] = '-';
	}
	else{
		digits[6] = ' ';
	}
	
	if(digits[26] == '1' && digits[27] == '0'){
		digits[12] = '+';
	}
	else if(digits[26] == '0' && digits[27] == '1'){
		digits[12] = '-';
	}
	else{
		digits[12] = ' ';
	}
	
	if(digits[28] == '1' && digits[29] == '0'){
		digits[18] = '+';
	}
	else if(digits[28] == '0' && digits[29] == '1'){
		digits[18] = '-';
	}
	else{
		digits[18] = ' ';
	}
}

void DSKYLogic::write9(uint16_t word){
	word = word >> 1;
	
	if((word & 0b0000000000000010) == 0)//bit 2 -> COMP ACT
		lamps[14] = false;
	else
		lamps[14] = true;
	
	if((word & 0b0000000000000100) == 0)//bit 3 -> UPLINK ACTY
		lamps[0] = false;
	else
		lamps[0] = true;
	
	if((word & 0b0000000000001000) == 0)//bit 4 -> TEMP
		lamps[7] = false;
	else
		lamps[7] = true;
	
	if((word & 0b0000000000010000) == 0)//bit 5 -> KEY REL
		lamps[3] = false;
	else
		lamps[3] = true;
	
	if((word & 0b0000000000100000) == 0){//bit 6 -> VERB / NOUN
		lamps[15] = false;
		lamps[17] = false;
	}
	else{
		lamps[15] = true;
		lamps[17] = true;
	}
	
	if((word & 0b0000000001000000) == 0)//bit 7 -> OPR ERR
		lamps[4] = false;
	else
		lamps[4] = true;
	
	//From there on bits are fictitious
	
	if((word & 0b0000000010000000) == 0)//bit 8 -> PROG DSP
		lamps[16] = false;
	else
		lamps[16] = true;
}

void DSKYLogic::write40(uint16_t word){
	if((word & 0b0000000000000100) == 0)
		nounBlinker = false;
	else
		nounBlinker = true;
	
	if((word & 0b0000000000000010) == 0)
		verbBlinker = false;
	else
		verbBlinker = true;
}
