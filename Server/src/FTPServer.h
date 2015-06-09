/*
 * FTPServer.h
 *
 *  Created on: Jun 8, 2015
 *      Author: jeffrey
 */

#ifndef SERVER_SRC_FTPSERVER_H_
#define SERVER_SRC_FTPSERVER_H_

#include "CFtpServer/CFtpServer.h"

class FTPServer {
public:

	void initialize();

	static FTPServer *getSingleton();

	int getPortNo();

private:

	void runServer(CFtpServer *FtpServer);

	bool nosockets;
	int portno;

	FTPServer();
	virtual ~FTPServer();

	static FTPServer singleton;
};

#endif /* SERVER_SRC_FTPSERVER_H_ */
