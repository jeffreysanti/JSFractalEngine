/*
 * main.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: jeffrey
 */


#include "stdio.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>
//#include "SocketClient.h"
#include "json/json.h"

#include "ConnectionManager.h"
#include "DirectoryManager.h"
#include "UserManager.h"
#include "FractalGen.h"





int main(int argc, char* argv[])
{
	DirectoryManager::getSingleton()->initialize(std::string(argv[0]));
	UserManager::getSingleton()->initialize();
	DBManager::getSingleton()->initialize();
	FractalGenTrackManager::getSingleton()->initialize();
	ConnectionManager::getSingleton()->initialize();

	FractalGen gen;

	//unsigned long start = time(NULL);
	while(true){
		gen.update();

		/*if(time(NULL) - start > 20){
			std::cout << "Time's up!\n";
			exit(0);
			return 0;
		}*/

		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return 0;
}



