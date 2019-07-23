/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#pragma once

#include <string.h>

using namespace std;

#define BLINKER_PERIOD 16666
#define STROBE_PERIOD 8333

class DSKYLogic
{
private:
	bool blinker;
	bool verbBlinker;
	bool nounBlinker;
	int blinkerCounter;
	int strobeCounter;
	bool lamps [18];
	char digits [31];//24 + 6 for sign + 1 fake
	
	char bitmapToDigit(uint8_t input);

public:
	DSKYLogic();
	void clearStrobes();
	string getStatus();
	void toggleBlinker();
	void write8(uint16_t word);
	void write9(uint16_t word);
	void write40(uint16_t word);
};