/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#include "agc.h"

agc::agc() {
		
	DSKYReady = false;
	dsky = DSKYLogic();
	boot();
	
}

void agc::boot(){
	
	// Clear memories
	for(int i=0; i<RAMSIZE; i++){
		RAM[i] = 0;
	}
	
	for(int i=0; i<ROMSIZE; i++){
		ROM[i] = 0;
	}
	
	for(int i=0; i<IOSIZE; i++){
		IO[i] = 0;
	}
	
	// Reset flags
	MASKINTR = false;
	INTR = false;
	EXT = false;
	OW = false;
	INX = false;
	
	// Load codes in memory
	loadIDTR();
	loadINT16();
	loadINT20();
	loadBIOS();
	loadMAIN();
	loadPrograms();
	//prog1();
	
	// Basic settings
	MCT = 0;
	T4INC = 0;
	TIME4 = 0xFFFE;
	time_zero = steady_clock::now();
	Z = (BIOS << 1);
	
}

void agc::rupt(){
	
	if(MASKINTR)
		return;
	
	switch(BRUPT >> 1){
		case KEY_RSET:
			if(verbose) cout << "Reboot machine\n";
			boot();
			break;
		default:
			return;
	}
	
}

void agc::exceptions(int e){
	MASKINTR = true;
			
	switch(e){
		
		case ACCESS_IN_IO_MEMORY:
			/*	
			 * E' stato effettuato un accesso in memoria di IO, con 
			 * un indirizzo non valido.
			 */
			cout << "\tEXCEPTION: ACCESS_IN_IO_MEMORY";
			break;
		
		case ACCESS_IN_ERASABLE_MEMORY:
			/*	
			 * E' stato effettuato un accesso in erasable memory (RAM), con 
			 * un indirizzo non valido.
			 */
			cout << "\tEXCEPTION: ACCESS_IN_ERASABLE_MEMORY";
			break;
			
		case ACCESS_IN_FIXED_MEMORY:
			/*	
			 * E' stato effettuato un accesso in fixed memory (ROM), con 
			 * un indirizzo non valido.
			 */
			cout << "\tEXCEPTION: ACCESS_IN_FIXED_MEMORY";
			break;
			
		case NO_DIVISION:
			/*	
			 * E' stata eseguita una divisione per zero.
			 */
			cout << "\tEXCEPTION: NO_DIVISION";
			break;
			
		case NO_INTERRUPT:
			/*	
			 * Non è stato individuato il gate di interruzione nella tabella 
			 * di interruzione.
			 */
			cout << "\tEXCEPTION: NO_INTERRUPT";
			break;
			
		case NO_OPERAND:
			/*	
			 * Non è stato decodificato un operando valido. 
			 */
			cout << "\tEXCEPTION: NO_OPERAND";
			break;
			
		case USED_EDITING_REGISTER:
			/*	
			 * E' stata effettuata una operazione non autorizzata su un registro di editing. 
			 */
			cout << "\tEXCEPTION: USED_EDITING_REGISTER";
			break;
			
		case ALT:
			/*	
			 * E' stata fermata la macchina.
			 */
			cout << "\tEXCEPTION: ALT";
			break;
			
		default:	
			/*	
			 * Si è verificata una situazione anomala. Non è stato possibile etichettare
			 * l'eccezione con una delle precedenti.
			 */
			cout << "\tEXCEPTION: UNKNOWN_ERROR";
	
	}
	
	debug();
	exit(-1);
}

void agc::dskyInput(uint16_t key){
	
	IO[12] = (IO[12] & 0b1111111111000001) | (key << 1); // Real AGC may not zero the bits before a new input if interrupt # has not been performed
	INT_TYPE = 20 << 1;
	setInterrupt();
	
}

string agc::getDSKYStatus(){
	return dsky.getStatus();
}

void agc::run(){
	DSKYReady = true;
}

void agc::resetProBit(){
	IO[25] = IO[25] & 0b1011111111111111;
}

void agc::setProBit(){
	IO[25] = IO[25] | 0b0100000000000000;
}

void agc::setMCT(uint16_t value){
	MCT += value;
}

void agc::slow_down(){
	
	auto now = steady_clock::now();
	long us = duration_cast<microseconds>(now - time_zero).count();
	long delta = MCT * CYCLE_PERIOD - us;
	if (delta > 1000){
		usleep(delta);
	}
	
}

uint16_t agc::loadWordIO(uint16_t addr){
	
	addr = addr >> 1; //One bit shift to the right to skip parity bit
	if(addr >= 512){
		cout << "[LOAD IO] Invalid memory access. Termination..." << endl;
		debug();
		exit(EXIT_FAILURE);
	}
	return IO[addr];

}

void agc::storeWordIO(uint16_t addr, uint16_t value){
	
	addr = addr >> 1; //One bit shift to the right to skip parity bit
	if(addr >= 512){
		cout << "[STORE IO] Invalid memory access. Termination..." << endl;
		debug();
		exit(EXIT_FAILURE);
	}
	
	value = checkOverflow(value);
	IO[addr] = value;
	
	if(addr == 8){
		dsky.write8(value);
	}
	if(addr == 9){
		dsky.write9(value);
	}
	if(addr == 40){
		dsky.write40(value);
	}
	
}

void agc::setOverflow(){
	OW = true;
}

void agc::unsetOverflow(){
	OW = false;
}

void agc::setExtended(){
	EXT = true;
}

void agc::unsetExtended(){
	EXT = false;
}

void agc::maskInterrupt(){
	unsetInterrupt();
	MASKINTR = true;
}

void agc::unmaskInterrupt(){
	MASKINTR = false;
}

void agc::setInterrupt(){
	INTR = true;
}

void agc::unsetInterrupt(){
	INTR = false;
}

void agc::setIndex(){
	INX = true;
}

void agc::unsetIndex(){
	if(INX){
		INX = false;
		ADDR += B;
	}
}

void agc::debug(){
	
	cout << "\n\tDEBUG\n" << endl;
	
	cout << "\t\t\tHEX\tDEC\t" << hex << endl; ;
	cout << "\tOPCODE:\t\t#\t" << dec << OPCODE << endl;
	cout << "\tADDRESS:\t" << hex << ADDR << '\t' << dec << (ADDR >> 1) << endl;
	
	cout << "\n\tFlags" << endl;
	cout << "\tMASKINTR:\t" << hex << "#" << '\t' << dec << MASKINTR << endl;
	cout << "\tINTR:\t\t" << hex << "#" << '\t' << dec << INTR << endl;
	cout << "\tEXT:\t\t" << hex << "#" << '\t' << dec << EXT << endl;
	cout << "\tINX:\t\t" << hex << "#" << '\t' << dec << INX << endl;
	cout << "\tOW:\t\t" << hex << "#" << '\t' << dec << OW << endl;
	
	cout << "\n\tMain registers" << endl;
	cout << "\tA:\t\t" << hex << A << '\t' << dec << (A >> 1) << endl;
	cout << "\tL:\t\t" << hex << L << '\t' << dec << (L >> 1) << endl;
	cout << "\tQ:\t\t" << hex << Q << '\t' << dec << (Q >> 1) << endl;
	cout << "\tZ:\t\t" << hex << Z << '\t' << dec << (Z >> 1) << endl;
	cout << "\tBB:\t\t" << hex << BB << '\t' << dec << (BB >> 1) << endl;
	
	cout << "\n\tInterrupt registers" << endl;
	cout << "\tARUPT:\t\t" << hex << ARUPT << '\t' << dec << (ARUPT >> 1) << endl;
	cout << "\tLRUPT:\t\t" << hex << LRUPT << '\t' << dec << (LRUPT >> 1) << endl;
	cout << "\tQRUPT:\t\t" << hex << QRUPT << '\t' << dec << (QRUPT >> 1) << endl;
	cout << "\tZRUPT:\t\t" << hex << ZRUPT << '\t' << dec << (ZRUPT >> 1) << endl;
	cout << "\tBRUPT:\t\t" << hex << BRUPT << '\t' << dec << (BRUPT >> 1) << "\n\n";
	cout << "\tBBRUPT:\t\t" << hex << BBRUPT << '\t' << dec << (BBRUPT >> 1) << "\n\n";
	
}

void agc::handlerIOAddress(){
	
	if(ADDR < (512 << 1))
		return;
	throw ACCESS_IN_IO_MEMORY;
	
}

void agc::handlerFixedMemAddress(){
	
	if(ADDR < (1024 << 1))
		throw ACCESS_IN_ERASABLE_MEMORY;
	
}

void agc::handlerErasableMemAddress(){
	
	if(ADDR < (1024 << 1))
		return;
	throw ACCESS_IN_FIXED_MEMORY;
	
}

void agc::loadIDTR(){
	
	// BIT PRESENZA			//	TIPO				ADDR INRERRUPT
	ROM[2048] = 1 << 1; 	ROM[2048+1] = 0 << 1; 	ROM[2048+3] = BIOS << 1;			// oct 2030 = 1048
	ROM[2048+4] = 0 << 1; 	ROM[2048+5] = 4 << 1;	ROM[2048+7] = T6RUPT << 1;
	ROM[2048+8] = 0 << 1;	ROM[2048+9] = 8 << 1;	ROM[2048+11] = T5RUPT << 1;
	ROM[2048+12] = 0 << 1;	ROM[2048+13] = 12 << 1;	ROM[2048+15] = T3RUPT << 1;
	
	ROM[2048+16] = 1 << 1;	ROM[2048+17] = 16 << 1;	ROM[2048+19] = T4RUPT << 1;
	ROM[2048+20] = 1 << 1; 	ROM[2048+21] = 20 << 1;	ROM[2048+23] = KEYRUPT1 << 1;
	
	ROM[2048+24] = 0 << 1;	ROM[2048+25] = 24 << 1;	ROM[2048+27] = KEYRUPT2 << 1;
	ROM[2048+28] = 0 << 1;	ROM[2048+29] = 28 << 1;	ROM[2048+31] = UPRUPT << 1;
	ROM[2048+32] = 0 << 1;	ROM[2048+33] = 32 << 1;	ROM[2048+35] = DOWNRUPT << 1;
	ROM[2048+36] = 0 << 1;	ROM[2048+37] = 36 << 1;	ROM[2048+39] = RADARRUPT << 1;
	ROM[2048+40] = 0 << 1;	ROM[2048+41] = 40 << 1;	ROM[2048+43] = HANDRUPT << 1;
	
	if(verbose) cout << "IDT built.\n";
	
}

void agc::loadIRegisters(){
	
	ARUPT = A;
	LRUPT = L;
	QRUPT = Q;
	ZRUPT = Z;
	BBRUPT = BB;
	
	if(verbose) cout << "Interrupting registers loaded\n";
	
}

void agc::loadInterrupt(){
	
	uint16_t addr = IDTR | INT_TYPE;
	uint16_t present = loadWord(addr);
	uint16_t type = loadWord(addr + 2);
	if(verbose) {
		cout << "addr present: " << addr << " >> " << ((addr)>>1) << endl;
		cout << "addr+1 type: " << (addr + (1 << 1)) << " >> " << ((addr + 2)>>1) << endl;
		cout << "addr+3 addr: " << (addr +  (3 << 1)) << " >> " << ((addr + 6)>>1) << endl;
		cout << "p: " << (present>>1) << endl;
		cout << "type: " << (type>>1) << endl;
	}
	if(present && type != INT_TYPE)
		throw NO_INTERRUPT;
	maskInterrupt();
	if(verbose) cout << "nuovo z: " << (loadWord(addr + 6)>>1) << endl;
	Z = loadWord(addr + 6);
	
}

int16_t agc::conv16(uint16_t a){
	
	uint32_t sign = a & 0x8000;
	a = a >> 1;
	if(sign)
		a = a + 1;
	a = a | sign;

	return (int16_t) a;
	
}

uint16_t agc::reconv16(int16_t a){
	
	uint16_t sign = a & 0x8000;
	a = a << 1;
	if(sign)
		a = a - 2;
		
	return (uint16_t) a;
	
}

int32_t agc::conv32(uint32_t a){

	uint32_t sign = a & 0x80000000;
	
	a = a >> 1;
																					// SDDDDDDDDDDDDDDP
	uint32_t aA = ((a & 0x7FFF0000) >> 2) | sign | (sign >> 1) | (sign >> 2);		// 0SDDDDDDDDDDDDDDP0000000000000000 (>> 2) 000SDDDDDDDDDDDDDDP0000000000000 (& 0x7FFE) 0000DDDDDDDDDDDDDD00000000000000 (| sign[]) SSSSDDDDDDDDDDDDDD00000000000000
	uint32_t aL = (a & 0x00003FFF);													// 00000000000000000SDDDDDDDDDDDDDD0 (>> 1) 00000000000000000SDDDDDDDDDDDDDD (& 0x3FFF) 000000000000000000DDDDDDDDDDDDDD
	
	a = aA | aL;
	if(sign)
		a = a + 1;
	
	return (int32_t) (a);
	
}

uint32_t agc::reconv32(int32_t a){
	
	uint32_t sign = (a & 0x80000000) >> 16;
	a = a << 1;

	if(sign)
		a = a - 2;
	
	L = (a & 0x00007FFE) | sign;
	A = ((a >> 14) & 0x00007FFE) | sign;	
	
	return ((A << 16) | L);
	
}

uint16_t agc::sum(uint16_t a, uint16_t b){
		
	int16_t as = conv16(a);
	int16_t bs = conv16(b);

	if((bs > 0 && as > INT15_MAX - bs) || (bs < 0 && as < INT15_MIN - bs)){
		if(verbose) cout << "Overflow!" << endl;
		setOverflow();
		SIGN = A & 0x8000;
	}
	
	return reconv16(as+bs);
	
}

uint16_t agc::sub(uint16_t a, uint16_t b){
		
	int16_t as = conv16(a);
	int16_t bs = conv16(b);
	

	if((bs < 0 && as > INT15_MAX + bs) || (bs > 0 && as < INT15_MIN + bs)){
		if(verbose) cout << "Overflow!" << endl;
		setOverflow();
		SIGN = A & 0x8000;
	}
		
	return reconv16(as-bs);
		
}

void agc::mul(uint16_t a, uint16_t b){
		
	int32_t as = conv16(a);
	int32_t bs = conv16(b);
	
	int32_t p = as * bs;
	
	if(((bs > 0 && as < INT29_MIN/bs) || (bs < -1 && as > INT29_MIN/bs) || (bs == -1 && as == INT29_MIN)) || ((bs > 0 && as > INT29_MAX/bs) || (bs < 0 && as < INT29_MAX/bs))){
		if(verbose) cout << "Overflow!" << endl;
		setOverflow();
		SIGN = A & 0x80000000;
	}
	
	reconv32(p);
	
}

void agc::div(uint16_t b){
		
	uint32_t A32 = (((uint32_t) A) << 16);
	uint32_t L32 = ((uint32_t) L);
	int32_t as = conv32(A32 | L32);
	int32_t bs = conv16(b);

	if(bs == 0)
		throw NO_DIVISION;
	
	int32_t dividend = (as / bs);
	int32_t remainder = (as % bs) & 0x0000EFFF;
	
	if((as == INT29_MIN) && (bs == -1)){
		cout << "Overflow!" << endl;
		setOverflow();
		SIGN = A & 0x80000000;
	}
	
	dividend = dividend  << 14;       			 //00000000000001
	
	int32_t d = dividend | remainder;
	
	reconv32(d);
	
}

uint16_t agc::loadWord(uint16_t addr){
	
	addr = addr >> 1;//One bit shift to the right to skip parity bit
	if(addr >= 4096){
		cout << "[LOAD] Invalid memory access. Termination..." << endl;
		debug();
		exit(EXIT_FAILURE);
	}
	
	if(addr < 1024){//RAM access				BIT 12-11 == 00
		if(addr < 768){//fixed RAM
			return RAM[addr];
		}
		else{//banked RAM
			uint16_t bankIndex = (EB & 0b0000111000000000) >> 9;
			return RAM[(bankIndex * 256) + (addr & 0b0000000011111111)];
		}
	}
	else{//ROM access							BIT 12-11 != 00
		if(addr < 2048){//banked ROM
			uint16_t bankIndex = (FB & 0b1111100000000000) >> 11;
			if(bankIndex >= 24){
				if(FEB & 0b0000000010000000){//Access to superbanks
					if(bankIndex < 28){
						cout << "Addr: " << addr  << endl;
						return ROM[((bankIndex + 8) * 1024) + (addr & 0b0000001111111111)];
					}
					else{
						return 0;//This area is wired to return 0
					}
				}
				else{
					return ROM[(bankIndex * 1024) + (addr & 0b0000001111111111)];
				}
			}
			else{
				return ROM[(bankIndex * 1024) + (addr & 0b0000001111111111)];
			}
		}
		else{//fixed ROM
			return ROM[addr];
		}
	}
	
}

uint16_t agc::checkOverflow(uint16_t value){
	
	if(OW){
		cout << "Correzione OW" << endl;
		cout << "Value: " << value << endl;
		value &= 0x7FFF;
		value |= SIGN;
		A &= 0x7FFF;
		A |= SIGN;
		if(SIGN == 0){
			value += 2;
			A += A;
		}else{
			value -= -2;
			A -= A;
		}
		OW = false;
	}
		
	return value;
	
}

void agc::storeWord(uint16_t addr, uint16_t value){
	
	addr = addr >> 1;//One bit shift to the right to skip parity bit
	if(addr >= 1024){
		cout << "[STORE MEM] Invalid memory access. Termination..." << endl;
		debug();
		exit(EXIT_FAILURE);
	}
	
	int RAMIndex;
	value = checkOverflow(value);
	
	if(addr < 768){//fixed RAM
		RAMIndex = addr;
	}
	else{//banked RAM
		uint16_t bankIndex = (EB & 0b0000111000000000) >> 9;
		RAMIndex = (bankIndex * 256) + (addr & 0b0000000011111111);
	}
	
	RAM[RAMIndex] = value;
	if(RAMIndex == 3)//Fix redundancy in BB
		RAM[6] = (value >> 8) & 0b0000000000000111;
	if(RAMIndex == 4)//Fix redundancy in BB
		RAM[6] = value & 0b1111100000000000;
	if(RAMIndex == 6){//Fix redundancy in EB and FB
		RAM[3] = (value << 8) & 0b0000111000000000;
		RAM[3] = value & 0b1111100000000000;
	}
	
	return;
	
}

void agc::isEditing(){
	
	if((ADDR >> 1) >= 20 && (ADDR >> 1) <= 23)
		throw USED_EDITING_REGISTER;
	
}

bool agc::getSign(uint16_t value){
	return value & 0x8000;
}

uint16_t agc::getValue(uint16_t value){
	return value & 0x7FFE;
}

void agc::fetch(uint16_t word){
	S = loadWord(word);
}

void agc::decode(uint16_t word){
	
	OPCODE = (word >> 13) & 0b00000111;
	
	if(!EXT){
		switch(OPCODE){
			case 0://XXALQ, XLQ, RETURN, RELINT, INHINT, EXTEND
				if(((word & 0b0001111111111110) >> 1) == 0){
					if(verbose) cout << "\tOPCODE:\t XXALQ\n";
					OPCODE = XXALQ;
					setMCT(1);
				}
				else if(((word & 0b0001111111111110) >> 1) == 1){
					if(verbose) cout << "\tOPCODE:\t XLQ\n";
					OPCODE = XLQ;
					setMCT(1);
				}
				else if(((word & 0b0001111111111110) >> 1) == 2){
					if(verbose) cout << "\tOPCODE:\t RETURN\n";
					OPCODE = RETURN;
					setMCT(2);
				}
				else if(((word & 0b0001111111111110) >> 1) == 3){
					if(verbose) cout << "\tOPCODE:\t RELINT\n";
					OPCODE = RELINT;
					setMCT(1);
				}
				else if(((word & 0b0001111111111110) >> 1) == 4){
					if(verbose) cout << "\tOPCODE:\t INHINT\n";
					OPCODE = INHINT;
					setMCT(1);
				}
				else if(((word & 0b0001111111111110) >> 1) == 6){
					if(verbose) cout << "\tOPCODE:\t EXTEND\n";
					OPCODE = EXTEND;
					setMCT(1);
				}
				else{
					if(verbose) cout << "\tOPCODE:\t TC\n";
					OPCODE = TC;
					ADDR = (word & 0b0001111111111110);
					setMCT(1);
				}
				break;
			case 1://CCS, TCF
				if(((word & 0b0001100000000000) >> 11) == 0){
					if(verbose) cout << "\tOPCODE:\t CCS\n";
					OPCODE = CCS;
					setMCT(2);
					ADDR = (word & 0b0000011111111110);
					handlerErasableMemAddress();
				}
				else{
					if(verbose) cout << "\tOPCODE:\t TCF\n";
					OPCODE = TCF;
					setMCT(1);
					ADDR = (word & 0b0001111111111110);
					handlerFixedMemAddress();
				}
				break;
			case 2://DAS, LXCH, INCR, ADS
				ADDR = (word & 0b0000011111111110);
				handlerErasableMemAddress();
				if(((word & 0b0001100000000000) >> 11) == 0){
					if(verbose) cout << "\tOPCODE:\t DAS\n";
					OPCODE = DAS;
					setMCT(3);
				}else if(((word & 0b0001100000000000) >> 11) == 1){
					if(verbose) cout << "\tOPCODE:\t LXCH\n";
					OPCODE = LXCH;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 2){
					if(verbose) cout << "\tOPCODE:\t INCR\n";
					OPCODE = INCR;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 3){
					if(verbose) cout << "\tOPCODE:\t ADS\n";
					OPCODE = ADS;
					setMCT(2);
				}
				break;
			case 3://CA
				OPCODE = CA;
				setMCT(2);
				ADDR = (word & 0b0001111111111110);
				if(ADDR == 0){
					if(verbose) cout << "\tOPCODE:\t NOOP\n";
				}else
					if(verbose) cout << "\tOPCODE:\t CA\n";
				break;
			case 4://CS
				if(verbose) cout << "\tOPCODE:\t CS\n";
				OPCODE = CS;
				setMCT(2);
				ADDR = (word & 0b0001111111111110);
				break;
			case 5://INDEX, DXCH, TS, XCH, RESUME
				ADDR = (word & 0b0000011111111110);
				handlerErasableMemAddress();
				if(((word & 0b0001100000000000) >> 11) == 0){
					if((ADDR >> 1) == 17){
						if(verbose) cout << "\tOPCODE:\t RESUME\n";
						OPCODE = RESUME;
					}else{
						if(verbose) cout << "\tOPCODE:\t INDEX\n";
						OPCODE = INDEX;
					}
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 1){
					if(verbose) cout << "\tOPCODE:\t DXCH\n";
					OPCODE = DXCH;
					setMCT(3);
				}else if(((word & 0b0001100000000000) >> 11) == 2){
					if(verbose) cout << "\tOPCODE:\t TS\n";
					OPCODE = TS;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 3){
					if(verbose) cout << "\tOPCODE:\t XCH\n";
					OPCODE = XCH;
					setMCT(2);
				}
				break;
			case 6://AD
				if(verbose) cout << "\tOPCODE:\t AD\n";
				OPCODE = AD;
				setMCT(2);
				ADDR = (word & 0b0001111111111110);
				break;
			case 7://MASK
				if(verbose) cout << "\tOPCODE:\t MASK\n";
				OPCODE = MASK;
				setMCT(2);
				ADDR = (word & 0b0001111111111110);
				break;
			default:
				throw NO_OPERAND;
		}
	}
	else{//Extended instructions
		switch(OPCODE){
			case 0://READ, WRITE, RAND, WAND, ROR, WOR, RXOR
				ADDR = (word & 0b0000001111111110);
				handlerIOAddress();
				if(((word & 0b0001110000000000) >> 10) == 0){
					if(verbose) cout << "\tOPCODE:\t READ\n";
					OPCODE = READ;
					setMCT(2);
				}
				else if(((word & 0b0001110000000000) >> 10) == 1){
					if(verbose) cout << "\tOPCODE:\t WRITE\n";
					OPCODE = WRITE;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 2){
					if(verbose) cout << "\tOPCODE:\t RAND\n";
					OPCODE = RAND;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 3){
					if(verbose) cout << "\tOPCODE:\t WAND\n";
					OPCODE = WAND;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 4){
					if(verbose) cout << "\tOPCODE:\t ROR\n";
					OPCODE = ROR;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 5){
					if(verbose) cout << "\tOPCODE:\t WOR\n";
					OPCODE = WOR;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 6){
					if(verbose) cout << "\tOPCODE:\t RXOR\n";
					OPCODE = RXOR;
					setMCT(2);
				}else if(((word & 0b0001110000000000) >> 10) == 7){
					if(verbose) cout << "\tOPCODE:\t ALT\n";
					OPCODE = ALT;
					setMCT(3);
				}
				break;
			case 1://DV, BZF
				if(((word & 0b0001100000000000) >> 11) == 0){
					if(verbose) cout << "\tOPCODE:\t DV\n";
					OPCODE = DV;
					setMCT(6);
					ADDR = (word & 0b0000011111111110);
				}
				else{
					if(verbose) cout << "\tOPCODE:\t BZF\n";
					OPCODE = BZF;
					setMCT(1);
					ADDR = (word & 0b0001111111111110);
					handlerFixedMemAddress();
				}
				break;
			case 2://MSU, QXCH, AUG, DIM
				ADDR = (word & 0b0000011111111110);
				handlerErasableMemAddress();
				if(((word & 0b0001100000000000) >> 11) == 0){
					if(verbose) cout << "\tOPCODE:\t MSU\n";
					OPCODE = MSU;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 1){
					if(verbose) cout << "\tOPCODE:\t QXCH\n";
					OPCODE = QXCH;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 2){
					if(verbose) cout << "\tOPCODE:\t AUG\n";
					OPCODE = AUG;
					setMCT(2);
				}else if(((word & 0b0001100000000000) >> 11) == 3){
					if(verbose) cout << "\tOPCODE:\t DIM\n";
					OPCODE = DIM;
					setMCT(2);
				}
				break;
			case 3://DCA
				if(verbose) cout << "\tOPCODE:\t DCA\n";
				OPCODE = DCA;
				setMCT(3);
				ADDR = (word & 0b0001111111111110);
				break;
			case 4://DCS
				if(verbose) cout << "\tOPCODE:\t DCS\n";
				OPCODE = DCS;
				setMCT(3);
				ADDR = (word & 0b0001111111111110);
				break;
			case 5://INDEX_EXTENDED
				if(verbose) cout << "\tOPCODE:\t INDEX_EXT\n";
				OPCODE = INDEX_EXT;
				ADDR = (word & 0b0001111111111110);
				break;
			case 6://SU, BZMF
				if(((word & 0b0001100000000000) >> 11) == 0){
					if(verbose) cout << "\tOPCODE:\t SU\n";
					OPCODE = SU;
					setMCT(2);
					ADDR = (word & 0b0000011111111110);
					handlerErasableMemAddress();
				}
				else{
					if(verbose) cout << "\tOPCODE:\t BZMF\n";
					OPCODE = BZMF;
					setMCT(1);
					ADDR = (word & 0b0001111111111110);
					handlerFixedMemAddress();
				}
				break;
			case 7://MP
				if(verbose) cout << "\tOPCODE:\t MP\n";
				OPCODE = MP;
				setMCT(3);
				ADDR = (uint)(word & 0b0001111111111110);
				break;
			default:
				throw NO_OPERAND;

		}
	}
	
}

int agc::exec(){
	
	uint16_t temp;
	
	unsetIndex();
	if(verbose) cout << "\tADDR:\t" << (ADDR >> 1) << endl;
	if(EXT == 0){
			
		switch(OPCODE){
		
			case XXALQ:															// TC A
				Q = Z;
				Z = 0xFFFE;	// Al termine del ciclo, (Z) = 65354 verrà incrementato e diventerà zero, facendo fetching nel primo registro.
				break;
						
			case XLQ:															// TC L
				Q = Z;
				Z = 0;
				break;
						
			case RETURN:
				Z = Q;
				break;
						
			case RELINT:
				unmaskInterrupt();
				break;
			
			case INHINT:
				maskInterrupt();
				break;
			
			case EXTEND:
				setExtended();
				break;
					
			case TC:
				Q = Z;
				Z = sub(ADDR, 2);
				break;
					
			case CCS:
				unsetOverflow();
				A = loadWord(ADDR);
				if(getSign(A) == 0){							
					if(getValue(A) > 0){										// (K) > +0
						A = sub(A,2);
					} else {													// (K) = +0	
						Z += 2;	// 0100 (>> 1) 2
						A = 0;
					} 
				} else {
					if(getValue(A) > 0){										// (K) < -0
						Z += 6;	// 0110 (>> 1) 3
						A = sub(getValue(A),2);
					} else {													// (K) = -0	
						Z += 8; // 1000 (>> 1) 4
						A = 0;
					}
				}
				break;
						
			case TCF:
				Z = sub(ADDR, 2);
				break;
				
			case DAS:
				unsetOverflow();
				A = sum(A, loadWord(ADDR));										// (DDOUBL) DAS A
				storeWord(ADDR, A);
				L = sum(L, loadWord(ADDR+2));
				storeWord(ADDR+2, L);
				break;
						
			case LXCH:
				if(ADDR == (7 << 1))											// (ZL) LXCH 7
					L = ZR;
				else{															// LXCH K
					temp = loadWord(ADDR);
					storeWord(ADDR, L);
					L = temp;
				}
				break;
						
			case INCR:
				storeWord(ADDR, sum(loadWord(ADDR), 2));
				break;
						
			case ADS:
				temp = loadWord(ADDR);
				A = sum(A, temp);
				storeWord(ADDR, A);
				break;
						
			case CA:
				if(ADDR == 0){													// CA A
					// NOOP
				}else{
					unsetOverflow();
					A = loadWord(ADDR);
				}
				break;
				
			case CS:
				unsetOverflow();
				if(ADDR == 0){
					A = ~A;														// (COM) CS A
					SIGN = (OW == 1) ? (~SIGN) : SIGN;
					
				}else{
					A = ~loadWord(ADDR);										// CS K
				}
				break;

			case RESUME:
				A = ARUPT;
				L = LRUPT;
				Q = QRUPT;
				BB = BBRUPT;
				Z = ZRUPT;
				unmaskInterrupt();
				break;
			
			case INDEX:
				setIndex();
				B = loadWord(ADDR);
				break;
						
			case DXCH:	// DXCH
				if((ADDR >> 1) == 1){											// DXCH L
					temp = Q;
					Q = L; 
					L = A;
					A = temp;
				} else if((ADDR >> 1) == 4){									// (DTCF) DXCH FB
					temp = A;
					A = FB; FB = temp;
					temp = L;
					L = Z; Z = temp;
				} else if ((ADDR >> 1) == 5){									// (DTCB) DXCH Z
					temp = A;
					A = Z; Z = temp;
					temp = L;
					L = BB; BB = temp;
				} else {														// DXCH K
					temp = A;
					A = loadWord(ADDR); 
					storeWord(ADDR, temp);
					temp = L;
					L = loadWord(ADDR+2); 
					storeWord(ADDR+2, temp);
				}
				break;
						
			case TS:															// TS
				if(OW && ADDR == 0){										// (OVSK) TS A
					Z += 2;
				} else if((ADDR >> 1) == 5){								// (TCAA) TS Z
					Z = A & 0x1FFE; 		// 0001 11..1 1110
					if(OW){
						if(SIGN)			// Positive ow
							A = 0x0002; 	// +1 = 0000 00..0 0010
						else				// Negative ow
							A = 0xFFFC; 	// -1 = 1111 11..1 1100
					Z += 2;
					}
				} else {													// TS K
					if(OW){
						storeWord(ADDR, SIGN | getValue(A));
						if(SIGN)			// Positive ow
							A = 0x0002; 	// +1 = 0000 00..0 0010
						else				// Negative ow
							A = 0xFFFC; 	// -1 = 1111 11..1 1100
						Z += 2;
					}else
						storeWord(ADDR, A);

				}
				unsetOverflow();
				break;
					
			case XCH:															// XCH
				temp = A;
				A = loadWord(ADDR); 
				storeWord(ADDR, temp);
				break;
				
			case AD:															// AD
				if(ADDR == 0)													// (DOUBLE) AD A
					A = sum(A, A);
				else 
					A = sum(A, loadWord(ADDR));
				break;
				
			case MASK:															// MASK
				A &= loadWord(ADDR);
				break;
			
			default:
				throw NO_OPERAND;
			
		}
		
	} else {
		
		unsetExtended();
		switch(OPCODE){
		
			case READ:	// READ
				A = loadWordIO(ADDR);
				break;
			
			case WRITE:	// WRITE
				storeWordIO(ADDR, A);
				break;
				
			case RAND:	// RAND
				A = A & loadWordIO(ADDR);
				break;
				
			case WAND:	// WAND
				A = A & loadWordIO(ADDR);
				storeWordIO(ADDR, A);
				break;
				
			case ROR:	// ROR
				A = A | loadWordIO(ADDR);
				break;
				
			case WOR:	// WOR
				storeWordIO(ADDR, A | loadWordIO(ADDR));
				break;
				
			case RXOR:	// RXOR
				A = A ^ loadWordIO(ADDR);
				break;
				
			case ALT: // ALT
				throw ALT;
				
			case DV:
				isEditing();
				unsetOverflow();
				div(loadWord(ADDR));	// controllare per eventuali ow
				break;
					
			case BZF:
				if(A == 0)
					Z = sub(ADDR, 2);
				break;
				
			case MSU:		// MSU
				unsetOverflow();
				A = reconv16((A >> 1) - (loadWord(ADDR) >> 1));		// Calcola la differenza tra due numeri in cmp2 e poi lo converte in cmp1
				break;
			
			case QXCH:		// QXCH
				if(ADDR == (7 << 1))														// (ZQ) QXCH 7
					Q = ZR;
				else{	
					temp = loadWord(ADDR);
					storeWord(ADDR, Q);
					Q = temp;
				}
				break;
				
			case AUG:		// AUG
				if(getSign(loadWord(ADDR)) == 0)
					storeWord(ADDR, sum(loadWord(ADDR), 2) );
				else
					storeWord(ADDR, sub(loadWord(ADDR), 2) );
				break;
				
			case DIM:		// DIM
				if(getSign(loadWord(ADDR)) == 0 && getValue(loadWord(ADDR)) > 0)
					storeWord(ADDR, sub(loadWord(ADDR), 2) );
				else if (getSign(loadWord(ADDR)) == 1 && getValue(loadWord(ADDR)) > 0)
					storeWord(ADDR, sum(loadWord(ADDR), 2) );
				break;
					
			case DCA:		// DCA
				unsetOverflow();
				A = loadWord(ADDR);
				L = loadWord(ADDR + 2);
				break;
				
			case DCS:		// DCS
				unsetOverflow();
				if(ADDR == 0){													// (DCOM) DCS A
					A = ~A;
					L = ~L;
				} else {
					A = ~loadWord(ADDR);
					L = ~loadWord(ADDR + 2);
				}
				break;
				
			case INDEX_EXT:	// INDEX
				setExtended();
				Z = sum(Z, sub(loadWord(ADDR), (1 << 1)));
				break;
				
			case SU:		// SU
				A = sub(A,loadWord(ADDR));
				break;
					
			case BZMF:		// BZMF
				if(getSign(A) == 1)
					Z = sub(loadWord(ADDR), (1 << 1));
				break;
				
			case MP:		// MP
				isEditing();
				unsetOverflow();
				if(ADDR == 0)													// (SQUARE) MP 0	
					mul(A, A);
				else
					mul(A, loadWord(ADDR));
				break;
		
			default:
				throw NO_OPERAND;
			
		}
		
	}

	return 0;
	
}

void agc::subroutine(){
	
	if((MCT / TIMER4_PERIOD) > T4INC){
		T4INC++;
		TIME4 = TIME4 + 2;
		if((TIME4 / 2) == 0){
			setInterrupt();
			INT_TYPE = 16 << 1;
		}
	}
	
}

void agc::interrupt(){
	
	if(INX == false && OW == false && EXT == false && MASKINTR == false && INTR == true){
		if(verbose) {
			cout << "RAM[600]: " << (RAM[600] >> 1) << endl;
			cout << "RAM[601]: " << (RAM[601] >> 1) << endl;
			cout << "RAM[602]: " << (RAM[602] >> 1) << endl;
			cout << "RAM[603]: " << (RAM[603] >> 1) << endl;
			cout << "busy: " << (RAM[604] >> 1) << endl;
			cout << "stato: " << (RAM[605] >> 1) << endl;
		}
		loadIRegisters();
		loadInterrupt();
		Z -= 2;				// Questa operazione, assieme al successivo incremento, rende invariato il registro Z
		
	}

}

void agc::specialroutine(){
	CYR = (CYR >> 1) | ((CYR & 0x0001) << 14);
	CYL = (CYL << 1) | ((CYL & 0x8000) >> 14);
	SR >>= 1;	
	ZR = 0;
	Z += 2;
	rupt();
}

int agc::emulate(){
	
	while(!DSKYReady);
	
	cout << "Emulation started.\n\n";
	
	if(verbose)
		debug();
		
	for(;;){
		if(verbose) cout << "Z: " << (Z >> 1) << endl;
		try{
			fetch(Z);
			decode(S);
			exec();
			subroutine();
			interrupt();
			specialroutine();
		}catch(int e){
			exceptions(e);
		}
		
		slow_down();
		
		dsky.toggleBlinker();
		dsky.clearStrobes();
		
	}
	
	return 0;
	
}

void agc::simulation(){
	
	cout << "Simulation started.\n\n";
	cout << "This execution try to compute just some istructions\n";
	
	try{
		
		fetch(Z);
		decode(S);
		exec();
		if(INTR){
			loadIRegisters();
			loadInterrupt();
		}

		debug();
		
		if(EXT){
			Z += 2;
			
			fetch(Z);
			decode(S);
			exec();
			if(INTR){
				loadIRegisters();
				loadInterrupt();
			}
			
			debug();
		}
	
	}catch(int e){
		
		exceptions(e);
		
	}
	
}