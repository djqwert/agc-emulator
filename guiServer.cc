/*
 *	Apollo Guidance Computer - Emulator
 *
 *  Authors: Alexander DeRoberto, Antonio Di Tecco 
 */

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

#include "guiServer.h"
#include "dskyConstants.h"

using namespace std;

int fetchRequest(char *buffer){
	const regex index_template("(GET \\/(index\\.html)? HTTP\\/\\d\\.\\d)");
	const regex status_template("(GET \\/status HTTP\\/\\d\\.\\d)");
	const regex keypress_template("(GET \\/button\\/[a-z0-9]{1} HTTP\\/\\d\\.\\d)");
	
	string request(buffer);

	smatch m;
	regex_search(request, m, index_template);
//	for(auto v: m) std::cout << v << std::endl;//DEBUG
	if(!m.empty() && m[0].matched){
//		cout << "Requested index.html" << endl;//DEBUG
		return 1;
	}
	
	regex_search(request, m, status_template);
//	for(auto v: m) std::cout << v << std::endl;//DEBUG
	if(!m.empty() && m[0].matched){
//		cout << "Status request" << endl;//DEBUG
		return 2;
	}
	
	regex_search(request, m, keypress_template);
//	for(auto v: m) std::cout << v << std::endl;//DEBUG
	if(!m.empty() && m[0].matched){
//		cout << "Key press" << endl;//DEBUG
		return 3;
	}
	
//	cout << "Sequence not found" << endl;//DEBUG
	return 0;
}

int getKey(char *buffer){
	const regex key_template("(GET \\/button\\/)([a-z0-9]{1})");
	
	string request(buffer);

	smatch m;
	regex_search(request, m, key_template);
	
//	for(auto v: m) std::cout << v << std::endl;//DEBUG
	if(!m.empty() && m[0].matched){
//		cout << "Key found" << endl;//DEBUG
		if(m[2].matched){
			if(m[2].str() == "0"){
				return KEY_0;
			}
			else if(m[2].str() == "1"){
				return KEY_1;
			}
			else if(m[2].str() == "2"){
				return KEY_2;
			}
			else if(m[2].str() == "3"){
				return KEY_3;
			}
			else if(m[2].str() == "4"){
				return KEY_4;
			}
			else if(m[2].str() == "5"){
				return KEY_5;
			}
			else if(m[2].str() == "6"){
				return KEY_6;
			}
			else if(m[2].str() == "7"){
				return KEY_7;
			}
			else if(m[2].str() == "8"){
				return KEY_8;
			}
			else if(m[2].str() == "9"){
				return KEY_9;
			}
			else if(m[2].str() == "a"){
				return KEY_ADD;
			}
			else if(m[2].str() == "s"){
				return KEY_SUB;
			}
			else if(m[2].str() == "v"){
				return KEY_VERB;
			}
			else if(m[2].str() == "n"){
				return KEY_NOUN;
			}
			else if(m[2].str() == "c"){
				return KEY_CLR;
			}
			else if(m[2].str() == "p"){
				return KEY_PRO_PRESS;
			}
			else if(m[2].str() == "z"){
				return KEY_PRO_RELEASE;
			}
			else if(m[2].str() == "k"){
				return KEY_KEY_REL;
			}
			else if(m[2].str() == "e"){
				return KEY_ENTR;
			}
			else if(m[2].str() == "r"){
				return KEY_RSET;
			}
		}
	}
		
	return 0;

}

void serverStart(agc &agc){
	int port = 8080;
	int server_fd, new_socket; 
	long valread;
	struct sockaddr_in address;
	int addrlen = sizeof(address);
	string response;

    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		cerr << "Error in GUI socket creation." << endl;
		exit(EXIT_FAILURE);
	}
	
	int option = 1;
	if(setsockopt(server_fd, SOL_SOCKET, (SO_REUSEPORT | SO_REUSEADDR), (char*)&option, sizeof(option)) < 0){
		cerr << "Error in GUI socket creation: setsockopt failed" << endl;
		close(server_fd);
		exit(EXIT_FAILURE);
	}
	

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );

	memset(address.sin_zero, '\0', sizeof address.sin_zero);

	if(bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0){
		cerr << "Error in GUI socket binding. Port " << port << " seems busy." << endl;
		exit(EXIT_FAILURE);
	}
	
	if(listen(server_fd, 10) < 0){
		cerr << "Error in GUI socket listen." << endl;
		exit(EXIT_FAILURE);
	}
	
	cout << "Type \"localhost:" << port << "/index.html\" in your browser to connect to the AGC emulator." << endl;
	while(1){
		if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
			cerr << "Error in GUI." << endl;
			exit(EXIT_FAILURE);
		}
		
		char buffer[1000] = {0};
		valread = read( new_socket , buffer, 1000);
		if(valread < 0)
			continue;
//		cout << buffer << endl;//DEBUG
		int requestType = fetchRequest(buffer);
		if(requestType == 1){
			string responseBody;
			ifstream file("./index.html");
			if(file){
				stringstream buffer;
				buffer << file.rdbuf();
				responseBody = buffer.str();
				file.close();
			}
			else{
				cerr << "Error: index.html is not available." << endl;
			}
			
			stringstream stringStream;
			stringStream << "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " << responseBody.length() << "\r\n\r\n" << responseBody;
			response = stringStream.str();
			//aggiungere eventuale ritardo
			agc.run();
		}
		else if(requestType == 2){
			string responseBody = agc.getDSKYStatus();
			stringstream stringStream;
			stringStream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << responseBody.length() << "\r\n\r\n" << responseBody;
			response = stringStream.str();
		}
		else if(requestType == 3){
			int keyPressed = getKey(buffer);
			
			if(keyPressed > 0 && keyPressed <= 100){
				agc.dskyInput((uint16_t)keyPressed);
				stringstream buffer;
				buffer << "{\x22success\x22:true,\x22key\x22:" << keyPressed << ",\x22message\x22:\x22Key " << keyPressed << " pressed\x22}";
				string responseBody = buffer.str();
				stringstream stringStream;
				stringStream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << responseBody.length() << "\r\n\r\n" << responseBody;
				response = stringStream.str();
			}
			else if(keyPressed == 101){
				agc.setProBit();
				stringstream buffer;
				buffer << "{\x22success\x22:true,\x22key\x22:" << keyPressed << ",\x22message\x22:\x22Key PRO pressed\x22}";
				string responseBody = buffer.str();
				stringstream stringStream;
				stringStream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << responseBody.length() << "\r\n\r\n" << responseBody;
				response = stringStream.str();
			}
			else if(keyPressed == 102){
				agc.resetProBit();
				stringstream buffer;
				buffer << "{\x22success\x22:true,\x22key\x22:" << keyPressed << ",\x22message\x22:\x22Key PRO released\x22}";
				string responseBody = buffer.str();
				stringstream stringStream;
				stringStream << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << responseBody.length() << "\r\n\r\n" << responseBody;
				response = stringStream.str();
			}
			else
				response = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 13\r\n\r\n{\"success\":false,\"message\":\"Bad key event\"}";
		}
		else{
			response = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\nContent-Length: 5\r\n\r\nError";
		}
		
		write(new_socket, &response[0], strlen(&response[0]));
		close(new_socket);
	}
}
