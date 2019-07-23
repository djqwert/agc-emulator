/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#include <iostream>
#include <thread>
#include <unistd.h>
#include <csignal>

#include "agc.h"
#include "guiServer.h"

using namespace std;

agc agc;
bool verbose = false;

void signalHandler( int signum ) {
   cout << "\nInterrupt signal (" << signum << ") received.\n";
   agc.debug();
   exit(-1);  
}

void usage(const char *name) {
	cerr << "Usage: " << name << " [-v]\n";
}

int main(int argc, char *argv[]){
	
	char ch;
	
	signal(SIGINT, signalHandler);
	
	while ( (ch = getopt(argc, argv, "advns:rb")) != -1) {
		switch (ch) {
			case 'v':
				verbose = true;
				break;
			default:
				usage(argv[1]);
				return 1;
		}
	}
	
	thread guiThread(serverStart, std::ref(agc));
	agc.emulate();
	guiThread.join();
	
	return 0;

}