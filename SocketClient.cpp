/*
 * SocketClient.cpp
 *
 *  Created on: May 27, 2014
 *      Author: jeffrey
 */

#include "SocketClient.h"

#define MAXSZ 33554432 /* 32MB */

extern void sendToAll(const char *dta, unsigned int len); // in main.cpp

SocketClient::SocketClient(int s, sockaddr_in a, FractalGen *g) {
	sock = s;
	gen = g;
	addr = a;
}

SocketClient::~SocketClient() {
}

#define MAX_BUF 1024

bool SocketClient::isAuthenticated(){
	if(u.id < 0)
		return false;
	return true;
}

bool SocketClient::update(){
	char *pkt;
	unsigned int len;
	char status = readPacket(&pkt, len);
	if(status == 2)
		return false; // disconnected
	if(status == 1)
		return true; // not ready yet

	// process packet now
	if(len < 4)
		return true; // ignore packet -- missing head

	std::string head = "";// + std::string(pkt[0]) + std::string(pkt[1]) + std::string(pkt[2]) + std::string(pkt[3]);
	head.append(1, pkt[0]); head.append(1, pkt[1]); head.append(1, pkt[2]); head.append(1, pkt[3]);

	processPacket(head, pkt+4);
	delete [] pkt;
	return true;
}

void SocketClient::close(){
#ifdef _WIN32
	closesocket(sock);
#else
	::close(sock);
#endif
}

void SocketClient::sendPacket(const char* dta, unsigned int len)
{
	unsigned long now = time(NULL); // spend no more than 3 sec sending
	if(len > 4){
		char tmp[4];
		memcpy(tmp, dta, 4);
		std::string head(tmp);
		std::cout << "SENT: " << head << "\n";
	}

	if(len == 0)
		return;
	int magicSend = htonl(MAGIC);
	int tmp = htonl(len);
	int sent=0;
	int s=0;

	while(sent < 4){
		s = send(sock, (const char*)(&magicSend+sent), sizeof(unsigned int)-sent, 0);
		if(s < 0)
			return;
		sent += s;
		if(time(NULL) > now + 3)
			return;
	}
	sent = 0;
	while(sent < 4){
		s = send(sock, (const char*)(&tmp+sent), sizeof(unsigned int)-sent, 0);
		if(s < 0)
			return;
		sent += s;
		if(time(NULL) > now + 3)
			return;
	}

	// now send data
	sent = 0;
	while(sent < len){
		s = send(sock, dta+sent, len-sent, 0);
		if(s < 0)
			return;
		sent += s;
		if(time(NULL) > now + 3)
			return;
	}
}

#ifdef _WIN32
#define wouldBeBlocked (WSAGetLastError() == WSAEWOULDBLOCK)
#else
#define wouldBeBlocked(i) (errno == EWOULDBLOCK || errno == EAGAIN)
#endif
#define isBlocked 

char SocketClient::readInt(unsigned int &val, bool block){
	unsigned long start = time(NULL);
	int i = 0;
	memset(&val, 0, 4);
	while(i < 4){ // read all four bytes
		int count = recv(sock,  (char*)(&val+i), 4-i, 0);
		if(count == 0){
			std::cout << "Client Disconnected\n";
			close();
			return 2; // disconnected
		}if(count < 0 && !wouldBeBlocked){
			std::cerr << "Client Disconnected Unusually: Errno: " << errno << "\n";
			close();
			return 2; // disconnected
		}else if(count < 0){ // no data avail [only report for first byte]
			if(!block && i==0)
				return 1;

			if(time(NULL) - start > 3){ // after 3 sec likely packet ain't coming
				std::cout << "Kicking Client for not sending complete packet in time\n";
				close();
				return 2;
			}
		}else{
			i += count;
		}
	}
	val = ntohl(val);
	return 0;
}

char SocketClient::readPacket(char **dta, unsigned int &len)
{
	// read the magic# and length
	unsigned int magic = 0;
	char status = readInt(magic, false);
	if(status != 0)
		return status; // either no data avail or connection lost

	if(magic != MAGIC){
		std::cout << "Kicking Client for packet w/o MAGIC ID\n";
		close();
		return 2;
	}

	len = 0;
	status = readInt(len);
	if(status != 0)
		return status; // either no data avail or connection lost

	if(len == 0)
		return 1; // no data actually received except header -> call busy

	if(len > MAXSZ){
		std::cout << "Kicking Client for sending packet over 32MB\n";
		close();
		return 2;
	}

	// now read packet data
	*dta = new char[len+1];
	unsigned long start = time(NULL);
	int i = 0;
	memset(*dta, 0, len);
	while(i < len){ // read all bytes
		int count = recv(sock, *dta+i, len-i, 0);
		if(count == 0){
			std::cout << "Client Disconnected\n";
			close();
			delete [] *dta;
			return 2; // disconnected
		}if(count < 0 && !wouldBeBlocked){
			std::cerr << "Client Disconnected Unusually: Errno: " << errno << "\n";
			close();
			delete [] *dta;
			return 2; // disconnected
		}else if(count < 0){ // no data avail [only report for first byte]
			if(time(NULL) - start > 3){ // after 3 sec likely packet ain't coming
				std::cout << "Kicking Client for not sending complete packet in time\n";
				close();
				delete [] *dta;
				return 2;
			}
		}else{
			i += count;
		}
	}
	(*dta)[len] = 0; // null terminate
	return 0;
}


void SocketClient::processPacket(std::string head, char *dta)
{
	std::string str = "";
	if(strlen(dta) > 0)
		str = std::string(dta);


	//std::cout << head << "->" << dta << "\n";
	//std::cout << "    '" << str << "'\n";
	std::cout << "RECV: " << head << " &&&: " << str << "\n";
	if(u.id < 0){ // only allow auth packets
		if(head == "AUTH"){       // USER AUTHENTICATION
			if(str.find(":") == str.npos){
				std::string toSend = "AUTH:NO";
				sendPacket(toSend.c_str(), toSend.length());
				return;
			}
			std::string user = str.substr(0, str.find(":"));
			std::string pass = str.substr(str.find(":")+1);
			u = gen->authenticateUser(user, pass);
			std::string toSend = "AUTH:NO";
			if(u.id >= 0){
				toSend = concat("AUTH:YES/", u.id);
				if(u.admin)
					toSend = concat("AUTH:YESADMIN/", u.id);
			}
			sendPacket(toSend.c_str(), toSend.length());
			return;
		}
		return;
	}
	if(head == "STAT"){ // report the status of the server
		std::string toSend = "STAT";
		toSend += gen->getJobQueue(u.id) + "#";
		std::vector<unsigned int> J = gen->currentJobIDs();
		for(int i=0; i<J.size(); i++){
			if(gen->ownerOfCurrentJob(J[i]) == u.id)
				toSend += concat("!",J[i])+",";
			else
				toSend += concat("",J[i])+",";
		}
		sendPacket(toSend.c_str(), toSend.length());
	}
	if(head == "SSLT"){ // search string list
		std::string toSend = "SSLT" + gen->getDB()->getJobSearchResult(str);
		sendPacket(toSend.c_str(), toSend.length());
	}
	if(head == "RJIF"){
		int jid = atoi(dta);
		char *resp = NULL;
		int len = 0;
		gen->fillMDUDRequest(jid, &resp, len);
		if(len > 0){
			sendPacket(resp, len);
			delete [] resp;
		}
	}
	if(head == "SFTI"){ // set fractal title
		if(str.find("|") != str.npos){
			int jid = atoi(str.substr(0, str.find("|")).c_str());
			str = str.substr(str.find("|") +1);
			FractalMeta f = gen->getDB()->getFractal(jid);
			if(f.jobID == jid && jid >= 0){
				f.name = str;
				gen->getDB()->updateFractal(f);
				char *resp = NULL;
				int len = 0;
				gen->fillMDUDRequest(jid, &resp, len);
				if(len > 0){
					sendToAll(resp, len);
					delete [] resp;
				}
			}

		}
	}
	if(head == "SFAU"){ // set fractal author
		if(str.find("|") != str.npos){
			int jid = atoi(str.substr(0, str.find("|")).c_str());
			str = str.substr(str.find("|") +1);
			FractalMeta f = gen->getDB()->getFractal(jid);
			if(f.jobID == jid && jid >= 0){
				f.author = str;
				gen->getDB()->updateFractal(f);
				char *resp = NULL;
				int len = 0;
				gen->fillMDUDRequest(jid, &resp, len);
				if(len > 0){
					sendToAll(resp, len);
					delete [] resp;
				}
			}

		}
	}
	if(head == "RCTX"){ // request context
		if(str.length() > 0){
			int jid = atoi(str.c_str());
			char *resp = NULL;
			int len = 0;
			gen->fillRCTXRequest(jid, &resp, len);
			if(len > 0){
				sendPacket(resp, len);
				delete [] resp;
			}
		}
	}
	if(head == "SJOB"){ // submit job
		if(str.length() > 0){
			FractalMeta f;
			f.userID = u.id;
			f.status = FDBS_QUEUED;
			Paramaters *p = new Paramaters();
			p->loadFromString(str);
			int jid = gen->postJob(p, u.id, u.daemon);
			std::string toSend = concat("SJOB", jid);
			sendPacket(toSend.c_str(), toSend.length());
		}
	}
	if(head == "CJOB"){ // cancel job
		if(str.length() > 0){
			int jid = atoi(str.c_str());
			gen->cancelJob(jid, u.id, u.admin);
		}
	}
}

void SocketClient::sendStatusUpdate()
{
	if(u.id < 0)
		return;

	std::string update = "UPDT";
	update += "Rendering: ";
	std::vector<unsigned int> J = gen->currentJobIDs();
	if(J.size() == 0)
		update += "<None>";
	else
		update += J[0];
	for(int i=1; i<J.size(); i++){
		update += ", "+J[i];
	}
	update += concat(" | ", gen->remainingJobs()) + std::string(" Job(s) Remain");
	update += concat(" ", time(NULL));

	sendPacket(update.c_str(), update.length());
}



