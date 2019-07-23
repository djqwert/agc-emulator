/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#pragma once

#include <chrono>
#include <iostream>
#include <cstdint>
#include <unistd.h>
#include <stdlib.h>
#include <bitset>
#include <string.h>
#include <sstream>

#include "agcConstants.h"
#include "dskyConstants.h"
#include "DSKYLogic.h"

using namespace std;
using namespace chrono;

extern bool verbose;

class agc {
	
private:
	
	// GUI
	bool DSKYReady;
	DSKYLogic dsky;
	
	// Work registers
	uint16_t OPCODE;
	uint16_t ADDR;
	const uint16_t IDTR = 0x1000;		// Registro della tabella di interruzione
	
	// Flag
	bool MASKINTR;				// Interruzione mascherata ON/OFF
	bool INTR;					// Si è verifica un'interruzione
	bool EXT;					// EXTENDED operation
	bool OW;					// Overflow flag
	bool INX;					// Index flag
	
	// Interrupt type
	uint16_t INT_TYPE;
	
	// Main structures
	uint16_t RAM[RAMSIZE];
	uint16_t ROM[ROMSIZE];
	uint16_t IO[IOSIZE];
	uint16_t& FEB = IO[7];
	uint16_t SIGN;
	
	// SIMULATE TIME
	long unsigned MCT;			// Durata dell'istruzione: 1 MCT = 12 us
	long unsigned T4INC;
	time_point<steady_clock> time_zero;

	uint16_t S;					// Registro non accessibile allo sviluppatore usato per controllare l'address (se è su 16 o 12 bit) ed accedere alla memoria
	uint16_t B;					// Usato per alcune operazioni e index opcode

	// CPU registers
	uint16_t& A = RAM[0]; 		// 0000
	uint16_t& L = RAM[1]; 		// 0001
	uint16_t& Q = RAM[2]; 		// 0002
	uint16_t& EB = RAM[3]; 		// 0003
	uint16_t& FB = RAM[4];		// 0004					11 0000 0000
	uint16_t& Z = RAM[5]; 		// 0005
	uint16_t& BB = RAM[6]; 		// 0006
	uint16_t& ZR = RAM[7]; 		// 0007-09
	
	// Interrupt registers
	uint16_t& ARUPT = RAM[10]; 	// 0010
	uint16_t& LRUPT = RAM[11]; 	// 0011
	uint16_t& QRUPT = RAM[12]; 	// 0012
	uint16_t& SAMPTIME = RAM[13];	// 0013-14
	uint16_t& ZRUPT = RAM[15]; 	// 0015
	uint16_t& BBRUPT = RAM[16]; // 0016		
	uint16_t& BRUPT = RAM[17]; 	// 0017		Registro contenente valore di ritorno dell'interrupt
	
	// Edit registers
	uint16_t& CYR = RAM[20]; 	// 0020
	uint16_t& SR = RAM[21]; 	// 0021
	uint16_t& CYL = RAM[22]; 	// 0022
	uint16_t& EDOP = RAM[23]; 	// 0023
	
	// Timing registers
	uint16_t& TIME1 = RAM[24]; 	// 0024
	uint16_t& TIME2 = RAM[25]; 	// 0025
	uint16_t& TIME3 = RAM[26]; 	// 0026
	uint16_t& TIME4 = RAM[27]; 	// 0027
	uint16_t& TIME5 = RAM[30]; 	// 0030
	uint16_t& TIME6 = RAM[31]; 	// 0031
	
	/* others registers... but not specified because not used */
	
	/* manage timing */
	void setMCT(uint16_t value);
	void slow_down();
	
public:
	
	agc();
	
	int emulate();
	void simulation();
	
	/* bios and programs */
	void boot();
	void loadBIOS();
	void loadMAIN();
	void loadPrograms();
	
	/* memory */
	void memoryTest(); 								/* debug function */
	uint16_t loadWordIO(uint16_t addr);				/* load a word from main memory */
	void storeWordIO(uint16_t addr, uint16_t value);/* store a word in the main memory */
	uint16_t loadWord(uint16_t addr);				/* load a word from main memory */
	void storeWord(uint16_t addr, uint16_t value);	/* store a word in the main memory */
	bool getSign(uint16_t value);					/* get sign from value */
	uint16_t getValue(uint16_t value);				/* get value removing sign */
	
	/* interrupt */
	void loadIDTR();
	void loadIRegisters();
	void loadInterrupt();
	void loadINT16();
	void loadINT20();
	void interrupt();
	void rupt();						/* interrupt return */
	
	/* flags */
	void setOverflow();
	uint16_t checkOverflow(uint16_t value);
	void unsetOverflow();
	void setExtended();
	void unsetExtended();
	void maskInterrupt();
	void unmaskInterrupt();
	void setInterrupt();
	void unsetInterrupt();
	void setIndex();
	void unsetIndex();
	
	/* handler exceptions */
	void handlerIOAddress();			/* check if addr is an io addr */
	void handlerErasableMemAddress();	/* check if addr is an erasable addr */
	void isEditing();					/* is an editing register? */
	void handlerFixedMemAddress();		/* check if addr is a fixed addr */
	void debug();						/* do machine diagnostics */
	void exceptions(int e);				/* manage exceptions and random behaviours */
	
	/* start execution using emulation */
	void fetch(uint16_t word);			/* fetches the next word and writes it in S register */
	void decode(uint16_t word);			/* decode the OPCODE and the ADDRESS in the respective variables */
	int exec();							/* execution of istruction */
	void subroutine();					/* manage execution of timers and others components, etc. */
	void specialroutine();				/* default instructions at each execution */

	/* conversions: used to convert number from 1's cmp to 2's cmp and make calculations and viceversa */
	int16_t conv16(uint16_t a);
	uint16_t reconv16(int16_t a);
	int32_t conv32(uint32_t a);
	uint32_t reconv32(int32_t a);
	
	/* calculations  */
	uint16_t sum(uint16_t a, uint16_t b);
	uint16_t sub(uint16_t a, uint16_t b);
	void mul(uint16_t a, uint16_t b);
	void div(uint16_t a);
	
	/* dsky */
	void dskyInput(uint16_t key);
	string getDSKYStatus();
	void run();
	void resetProBit();
	void setProBit();
	
	/* basic trial software */
	void prog1();
	
};
