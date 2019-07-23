/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#pragma once

#define RAMSIZE 8 * 256

#define KEY_0			16
#define KEY_1			1
#define KEY_2			2
#define KEY_3			3
#define KEY_4			4
#define KEY_5			5
#define KEY_6			6
#define KEY_7			7
#define KEY_8			8
#define KEY_9			9
#define KEY_VERB		17
#define KEY_RSET		18
#define KEY_KEY_REL		25
#define KEY_ADD			26
#define KEY_SUB			27
#define KEY_ENTR		28
#define KEY_CLR			30
#define KEY_NOUN		31
#define KEY_PRO_PRESS	101//particolare (agisce su un bit a parte)	https://www.ibiblio.org/apollo/developer.html#Table_of_IO_Channels
#define KEY_PRO_RELEASE	102

#define DSKY_BLANK		0
#define DSKY_0			21	// 0b10101
#define DSKY_1			3	// 0b00011
#define DSKY_2			25	// 0b11001
#define DSKY_3			27 	// 0b11011
#define DSKY_4			15	// 0b01111
#define DSKY_5			30	// 0b11110
#define DSKY_6			28	// 0b11100
#define DSKY_7			19	// 0b10011
#define DSKY_8			29	// 0b11101
#define DSKY_9			31	// 0b11111
