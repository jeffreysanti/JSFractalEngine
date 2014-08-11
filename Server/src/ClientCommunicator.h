/*
 * ClientCommunicator.h
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#ifndef CLIENTCOMMUNICATOR_H_
#define CLIENTCOMMUNICATOR_H_

#include <cstdint>
#include <deque>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#endif

#include <iostream>
#include <cerrno>
#include <cassert>

#include "DirectoryManager.h"
#include "UserManager.h"
#include "DBManager.h"

#define MAGIC   0xFA42AB
#define MAXSZ 33554432 /* 32MB */


struct PacketHeader{
	uint32_t magic;
	uint32_t len;
	uint32_t replyAddr;
	char pckType[4];
};

struct Packet{
	Packet(){
		data = nullptr;
	}
	PacketHeader hdr;
	char *data;
};



#ifdef _WIN32
#define wouldBeBlocked (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#define wouldBeBlocked (errno == EWOULDBLOCK || errno == EAGAIN)
#endif
#define isBlocked



class ClientCommunicator {
public:

#ifdef _WIN32
	ClientCommunicator(SOCKET s, unsigned int id);
#else
	ClientCommunicator(int s, unsigned int id);
#endif
	ClientCommunicator();


	virtual ~ClientCommunicator();

	bool hasNextPacket(std::string &head, unsigned int &len, unsigned int &rpt, char **data);
	unsigned int getID();

	void addPacketToQueue(std::string head, unsigned int len, char *data, unsigned int replyTo=0);
	void addPacketToQueue(std::string head, std::string str, unsigned int replyTo=0);

	void sendNoSocketAuthorization();

	bool isNoSockets();

	bool update();

private:

	std::deque<Packet*> O;
	std::deque<Packet*> R;

	Packet *Pin=nullptr; // currently receiving packet
	Packet *Pout=nullptr; // currently receiving packet

	unsigned int bytesIn;
	unsigned int bytesOut;

	unsigned int ID;

	void close();

	void deletePacket(Packet *pkt);

#ifdef _WIN32
	SOCKET sock;
#else
	int sock;
#endif
	bool nosocket;
};

#endif /* CLIENTCOMMUNICATOR_H_ */
