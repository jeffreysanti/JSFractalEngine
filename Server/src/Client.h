/*
 * SocketClient.h
 *
 *  Created on: May 27, 2014
 *      Author: jeffrey
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include "FractalGen.h"
#include "ClientCommunicator.h"

class Client {
public:
	Client(ClientCommunicator *com);
	virtual ~Client();

	bool updateClient();

	void handlePacket(std::string &head, unsigned int &len, unsigned int &replyTo,
						char *data, std::string &str);


	void writeServerMessage(std::string msg);
	void writeServerFractalUpdate(unsigned int jid);

private:

	ClientCommunicator *C;
	User U;


	// returns false if client has disconnected
	/*bool update();
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

	User u;*/
};

#endif /* SOCKETCLIENT_H_ */
