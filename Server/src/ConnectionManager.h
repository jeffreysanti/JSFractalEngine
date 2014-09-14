/*
 * ConnectionManager.h
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#ifndef CONNECTIONMANAGER_H_
#define CONNECTIONMANAGER_H_

#include <thread>
#include <chrono>
#include <iostream>
#include "DirectoryManager.h"
#include "Client.h"

class ConnectionManager {
public:

	void initialize();


	static ConnectionManager *getSingleton();

private:
	ConnectionManager();
	~ConnectionManager();



#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif


	std::thread *networkThread;


	static void threadMain();
	static ConnectionManager singleton;
	bool nosockets;
	unsigned int maxConnections;

	unsigned int nextClientID;

	std::vector<Client*> C;
};

#endif /* CONNECTIONMANAGER_H_ */
