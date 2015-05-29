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
#include "ParamsFile.h"
#include "FractalGen.h"



int manualSubmit(ParamsFile *p){
	FractalMeta f;
	f.userID = 0;
	f.status = FDBS_QUEUED;
	f.manualQueue = true;
	int jid = DBManager::getSingleton()->submitJob(f, p);
	return jid;
}


int main(int argc, char* argv[])
{
	std::cout << "JSFractalEngine Server Build: " << __DATE__ << " : " << __TIME__ << "\n";

	DirectoryManager::getSingleton()->initialize(std::string(argv[0]));
	SchemaManager::getSingleton()->initialize();
	UserManager::getSingleton()->initialize();
	DBManager::getSingleton()->initialize();
	FractalGenTrackManager::getSingleton()->initialize();
	ConnectionManager::getSingleton()->initialize();

	FractalGen gen;

	ParamsFile *p = new ParamsFile("test.job", true);
	//manualSubmit(p);


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



