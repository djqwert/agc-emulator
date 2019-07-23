/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

#pragma once

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <cstring>
#include <fstream>
#include <sstream>
#include <regex>

#include "agc.h"

using namespace std;

int fetchRequest(char *buffer);
int getKey(char *buffer);
void serverStart(agc &agc);