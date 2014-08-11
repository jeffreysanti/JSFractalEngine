/*
 * SocketClient.cpp
 *
 *  Created on: May 27, 2014
 *      Author: jeffrey
 */

#include "Client.h"


Client::Client(ClientCommunicator *com)
{
	C = com;
}

Client::~Client()
{
	delete C;
}

bool Client::updateClient(){
	if(!C->update())
		return false;

	std::string head;
	char *data;
	unsigned int len;
	unsigned int replyTo;

	while(C->hasNextPacket(head, len, replyTo, &data)){
		std::string str = "";

		if(data != nullptr){
			char *dta2 = new char[len+1];
			dta2[len] = 0;
			memcpy(dta2, data, len);
			str = std::string(dta2);
			delete [] dta2;
		}

		if(head == "AUTH" && C->isNoSockets()){
			std::cout << "Client " << C->getID() << " Requested Authentication\n";
			std::cout << "   Granted [Admin Rights]  [NoSocket AUTH]\n";
			std::cout << "   Priority Queue\n";
			C->sendNoSocketAuthorization();
			U = UserManager::getSingleton()->getNoSocketsAccount();
		}
		else if(head == "AUTH"){
			std::cout << "Client " << C->getID() << " Requested Authentication\n";
			if(str.find(":") == str.npos){
				C->addPacketToQueue("AUTH", "NO", replyTo);
				std::cout << "   Denied -> Invalid Request\n";
			}else{
				std::string user = str.substr(0, str.find(":"));
				std::string pass = str.substr(str.find(":")+1);
				U = UserManager::getSingleton()->getByName(user);
				if(U.name == user && U.pass == pass && U.id >= 0){
					if(U.admin){
						C->addPacketToQueue("AUTH", concat("YESADMIN/",U.id), replyTo);
						std::cout << "   Granted [Admin Rights]\n";
					}else{
						C->addPacketToQueue("AUTH", concat("YES/",U.id), replyTo);
						std::cout << "   Granted [Standard Access]\n";
					}
					if(U.daemon)
						std::cout << "   Daemon Queue\n";
					else
						std::cout << "   Priority Queue\n";
				}else{
					C->addPacketToQueue("AUTH", "NO", replyTo);
					std::cout << "   Denied -> Invalid Login\n";
				}
			}
		}else{
			if(U.id >= 0){ // user logged in
				handlePacket(head, len, replyTo, data, str);
			}
		}

		if(data != nullptr)
			delete []data;
	}

	if(!C->update()) // Immediately send any replies
		return false;
	return true;
}



void Client::handlePacket(std::string &head, unsigned int &len, unsigned int &replyTo,
						char *data, std::string &str)
{
	if(head == "SSLT"){
		std::string toSend = DBManager::getSingleton()->getJobSearchResult(str);
		C->addPacketToQueue("SSLT", toSend, replyTo);
	}else if(head == "RJIF"){
		int jid = atoi(str.c_str());
		char *resp = nullptr;
		int len = 0;
		DBManager::getSingleton()->fillMDUDRequest(jid, &resp, len);
		if(len > 0){
			C->addPacketToQueue("MDUD", len, resp, replyTo);
		}

	}else if(head == "RCTX"){
		int jid = atoi(str.c_str());
		char *resp = nullptr;
		int len = 0;
		DBManager::getSingleton()->fillRCTXRequest(jid, &resp, len);
		if(len > 0){
			C->addPacketToQueue("RCTX", len, resp, replyTo);
		}

	}else if(head == "STAT"){
		std::string toSend = "";
		std::vector<JobStatus> j;

		j = FractalGenTrackManager::getSingleton()->getQueuedJobs();
		for(auto it=j.begin(); it!=j.end(); it++){
			if((*it).owner == U.id)
				toSend = concat(toSend+",U",(*it).jid);
			else
				toSend = concat(toSend+",",(*it).jid);
		}
		if(toSend.length() > 0)
			toSend = toSend.substr(1); // remove first comma
		toSend += "#";
		j = FractalGenTrackManager::getSingleton()->getCurrentJobs();
		for(auto it=j.begin(); it!=j.end(); it++){
			if((*it).owner == U.id)
				toSend += concat("!",(*it).jid)+",";
			else
				toSend += concat("",(*it).jid)+",";
		}
		if(toSend.at(toSend.length()-1) == ',')
			toSend = toSend.substr(0, toSend.length()-1); // remove last comma
		C->addPacketToQueue("STAT", toSend, replyTo);
	}else if(head == "SJOB"){ // submit job
		if(str.length() > 0){
			FractalMeta f;
			f.userID = U.id;
			f.status = FDBS_QUEUED;
			f.manualQueue = !U.daemon;
			Paramaters *p = new Paramaters();
			p->loadFromString(str);
			int jid = DBManager::getSingleton()->submitJob(f, p);
			C->addPacketToQueue("SJOB", concat("",jid), replyTo);
		}
	}else if(head == "CJOB"){ // cancel job
		if(str.length() > 0){
			unsigned int jid = atoi(str.c_str());
			FractalMeta meta = DBManager::getSingleton()->getFractal(jid);
			if(meta.jobID==jid && (U.admin || U.id==meta.userID)){
				InfoExchange ench;
				ench.type = EXT_AFFECT_RENDERING_FRACTAL;
				ench.artType = ART_CANCEL;
				ench.uiParam1 = jid;
				FractalGenTrackManager::getSingleton()->postExchange(ench);
			}
		}
	}else if(head == "SFTI"){ // set title
		if(str.find("|") != str.npos){
			int jid = atoi(str.substr(0, str.find("|")).c_str());
			str = str.substr(str.find("|") +1);

			FractalMeta meta = DBManager::getSingleton()->getFractal(jid);
			if(meta.jobID==jid && (U.admin || U.id==meta.userID)){
				InfoExchange ench;
				ench.type = EXT_AFFECT_RENDERING_FRACTAL;
				ench.artType = ART_CHANGE_NAME;
				ench.uiParam1 = jid;
				ench.sParam3 = str;
				FractalGenTrackManager::getSingleton()->postExchange(ench);
			}
		}
	}else if(head == "SFAU"){ // set author
		if(str.find("|") != str.npos){
			int jid = atoi(str.substr(0, str.find("|")).c_str());
			str = str.substr(str.find("|") +1);

			FractalMeta meta = DBManager::getSingleton()->getFractal(jid);
			if(meta.jobID==jid && (U.admin || U.id==meta.userID)){
				InfoExchange ench;
				ench.type = EXT_AFFECT_RENDERING_FRACTAL;
				ench.artType = ART_CHANGE_AUTHOR;
				ench.uiParam1 = jid;
				ench.sParam3 = str;
				FractalGenTrackManager::getSingleton()->postExchange(ench);
			}
		}
	}else{
		std::cout << "Lost Packet Received. HEAD: " << head << "\n";
	}
}


void Client::writeServerMessage(std::string msg)
{
	if(U.id < 0) // do not send to unauthenticated clients
		return;

	C->addPacketToQueue("UPDT", msg, 0);
}

void Client::writeServerFractalUpdate(unsigned int jid)
{
	char *resp = nullptr;
	int len = 0;
	DBManager::getSingleton()->fillMDUDRequest(jid, &resp, len);
	if(len > 0){
		C->addPacketToQueue("MDUD", len, resp, 0);
	}
}

