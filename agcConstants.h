/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#pragma once

// STANDARD ISA
#define XXALQ 		0
#define XLQ			1
#define RETURN		2
#define RELINT 		3
#define INHINT 		4
#define EXTEND 		5
#define TC			6
#define	CCS			7
#define TCF 		8
#define DAS 		9
#define LXCH 		10
#define INCR 		11
#define ADS 		12
#define CA 			13
#define CS 			14
#define RESUME		15
#define INDEX 		16
#define DXCH		17
#define TS			18
#define XCH			19
#define AD 			20
#define MASK		21

// EXTENDED ISA
#define READ 		22
#define WRITE 		23
#define RAND 		24
#define WAND 		25
#define ROR 		26
#define WOR 		27
#define RXOR 		28
#define DV 			29
#define BZF 		30
#define MSU 		31
#define QXCH 		32
#define AUG			33
#define DIM 		34
#define DCA 		35
#define DCS 		36
#define INDEX_EXT	37
#define SU 			38
#define BZMF 		39
#define MP 			40
#define ALT			41

// MEM DIMENSIONS
#define RAMSIZE 	8 * 256
#define ROMSIZE 	36 * 1024
#define IOSIZE 		512

// RANGE VALUES
#define INT15_MIN	-16383
#define INT15_MAX	+16383
#define INT29_MIN	-268435455
#define INT29_MAX	+268435455

// INTERRUPT ADDRESSES
#define BIOS		0x082C // 000100000101100 2092
#define T6RUPT		1
#define T5RUPT		5
#define T3RUPT		1
#define T4RUPT		0x0BBA // 000101110111010‬ 3002
#define KEYRUPT1	0x0935 // 2357 // ‭000100111000100‬ 2500 0x09C4
#define KEYRUPT2	1
#define UPRUPT		1
#define DOWNRUPT	1
#define RADARRUPT	1
#define HANDRUPT	1

// EXCEPTIONS
#define ACCESS_IN_IO_MEMORY			0
#define ACCESS_IN_ERASABLE_MEMORY	1
#define ACCESS_IN_FIXED_MEMORY		2
#define NO_DIVISION					3
#define NO_INTERRUPT				4
#define NO_OPERAND					5
#define USED_EDITING_REGISTER		6

// TIME
#define CYCLE_PERIOD 12 //in microseconds
#define TIMER4_PERIOD (10000 / 12) // 10ms
