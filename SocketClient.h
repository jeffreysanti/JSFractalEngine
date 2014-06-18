/*
 * SocketClient.h
 *
 *  Created on: May 27, 2014
 *      Author: jeffrey
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

#include "FractalGen.h"

#define MAGIC   0xFA42AB

class SocketClient {
public:
	SocketClient(int s, sockaddr_in a, FractalGen *g);
	virtual ~SocketClient();

	// returns false if client has disconnected
	bool update();
	void close();

	void sendStatusUpdate();

	void sendPacket(const char *dta, unsigned int len);

	bool isAuthenticated();


private:

	// 0 on success, 1 on busy, 2 on disconnect
	char readPacket(char **dta, unsigned int &len);
	char readInt(unsigned int &val, bool block=true);


	void processPacket(std::string head, char *dta);

	int sock;
	FractalGen *gen;
	sockaddr_in addr;

	User u;
};

#endif /* SOCKETCLIENT_H_ */
