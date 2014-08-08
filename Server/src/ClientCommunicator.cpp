/*
 * ClientCommunicator.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#include "ClientCommunicator.h"

#ifdef _WIN32
ClientCommunicator::ClientCommunicator(SOCKET s, unsigned int id)
{
	sock = s;
	nosocket = false;
	bytesIn = bytesOut = 0;
	Pin = new Packet;
	Pout = nullptr;
	ID = id;
}
#else
ClientCommunicator::ClientCommunicator(int s, unsigned int id)
{
	sock = s;
	nosocket = false;
	bytesIn = bytesOut = 0;
	Pin = new Packet;
	Pout = nullptr;
	ID = id;
}
#endif
ClientCommunicator::ClientCommunicator()
{
	nosocket = true;
	sock = 0;
	bytesIn = bytesOut = 0;
	ID = 0;
	Pin = new Packet;
	Pout = nullptr;
}

ClientCommunicator::~ClientCommunicator() {
	deletePacket(Pin);
	deletePacket(Pout);
	Pin = nullptr;
	Pout = nullptr;

	for(auto it=O.begin(); it != O.end(); it++){
		deletePacket((*it));
		(*it) = nullptr;
	}
	O.clear();
	for(auto it=R.begin(); it != R.end(); it++){
		deletePacket((*it));
		(*it) = nullptr;
	}
	R.clear();
}

void ClientCommunicator::deletePacket(Packet *pkt)
{
	if(pkt != nullptr){
		if(pkt->data != nullptr){
			delete [] pkt->data;
			pkt->data = nullptr;
		}
		delete pkt;
		pkt = nullptr;
	}
}

unsigned int ClientCommunicator::getID()
{
	return ID;
}

bool ClientCommunicator::isNoSockets()
{
	return nosocket;
}

bool ClientCommunicator::hasNextPacket(std::string &head, unsigned int &len, unsigned int &rpt, char **data)
{
	if(R.empty())
		return false;

	Packet *pkt = R.front();
	R.pop_front();

	head = "";
	head.append(1, pkt->hdr.pckType[0]); head.append(1, pkt->hdr.pckType[1]);
	head.append(1, pkt->hdr.pckType[2]); head.append(1, pkt->hdr.pckType[3]);

	len = pkt->hdr.len;
	rpt = pkt->hdr.replyAddr;
	*data = nullptr;
	if(len > 0){
		*data = new char[len];
		memcpy(*data, pkt->data, len);
	}

	deletePacket(pkt);
	return true;
}

void ClientCommunicator::addPacketToQueue(std::string head, unsigned int len, char *data, unsigned int replyTo)
{
	Packet *pkt = new Packet;
	pkt->data = data;
	pkt->hdr.magic = htonl(MAGIC);
	pkt->hdr.len = htonl(len);
	pkt->hdr.replyAddr = htonl(replyTo);
	memcpy(&pkt->hdr.pckType, head.c_str(), 4);
	O.push_back(pkt);
}

void ClientCommunicator::addPacketToQueue(std::string head, std::string str, unsigned int replyTo)
{
	int len = str.length();
	char *data = nullptr;
	if(len > 0){
		data = new char[len];
		memcpy(data, str.c_str(), len);
	}

	addPacketToQueue(head, len, data, replyTo);
}



bool ClientCommunicator::update()
{
	if(!nosocket){
		// need to read if data has been sent
		if(bytesIn < 16){ //we are reading the header still
			int count = recv(sock, (char*)(Pin)+bytesIn, 16-bytesIn, 0);
			if(count == 0){
				std::cout << "Client Disconnected [" << ID << "]\n";
				close();
				return false;
			}
			if(count < 0 && !wouldBeBlocked){
				std::cout << "Client Disconnected [" << ID << "]\n";
				close();
				return false;
			}
			if(count > 0){
				bytesIn += count;
			}
			if(bytesIn == 16){ // header fully read
				// fix byte ordering
				Pin->hdr.magic = ntohl(Pin->hdr.magic);
				Pin->hdr.len = ntohl(Pin->hdr.len);
				Pin->hdr.replyAddr = ntohl(Pin->hdr.replyAddr);

				// verify integrity
				if(Pin->hdr.magic != MAGIC){
					std::cout << "Kicking Client " << ID << " for packet w/o MAGIC ID\n";
					close();
					return false;
				}

				if(Pin->hdr.len > MAXSZ){
					std::cout << "Kicking Client " << ID << " for sending packet over 32MB\n";
					close();
					return false;
				}
				if(Pin->hdr.len > 0){
					assert(Pin->data == nullptr);
					Pin->data = new char[Pin->hdr.len];
					*Pin->data = 'B';
				}
			}
		}
		if(bytesIn >= 16 && Pin->hdr.len > 0){ // read data section
			char* ptr = ((char*)Pin->data)+(bytesIn-16);
			int count = recv(sock, ptr, (Pin->hdr.len+16)-bytesIn, 0);
			if(count == 0){
				std::cout << "Client Disconnected [" << ID << "]\n";
				close();
				return false;
			}
			if(count < 0 && !wouldBeBlocked){
				std::cout << "Client Disconnected [" << ID << "]\n";
				close();
				return false;
			}
			if(count > 0){
				bytesIn += count;
			}
			if(bytesIn == Pin->hdr.len + 16){ // entire packet read -> reset reader
				R.push_back(Pin);
				Pin = new Packet;
				bytesIn = 0;
			}
		}else if(bytesIn == 16 && Pin->hdr.len == 0){
			R.push_back(Pin);
			Pin = new Packet;
			bytesIn = 0;
		}

		// Now let's send some data out
		if(Pout == nullptr && O.size() > 0){
			Pout = O.front();
			O.pop_front();
			bytesOut = 0;
		}
		if(Pout != nullptr){
			if(bytesOut < 16){ // send out header
				int count = send(sock, (char*)(Pout)+bytesOut, 16-bytesOut, 0);
				if(count < 0 && !wouldBeBlocked){
					std::cout << "Client Write Failed [" << ID << "] Kicking...\n";
					std::cout << "  CASE A" << errno << "\n";
					close();
					return false;
				}
				if(count > 0){
					bytesOut += count;
				}
			}
			if(bytesOut == 16){ // change length back to host to compare later
				Pout->hdr.len = ntohl(Pout->hdr.len);
			}
			if(bytesOut >= 16 && Pout->hdr.len > 0){ // write data section
				ssize_t count = send(sock, Pout->data+(bytesOut-16), Pout->hdr.len-(bytesOut-16), 0);
				if(count < 0 && !wouldBeBlocked){
					std::cout << "Client Write Failed [" << ID << "] Kicking...\n";
					std::cout << "  CASE B" << errno << "\n";
					close();
					return false;
				}
				if(count > 0){
					bytesOut += count;
				}
			}
			if(bytesOut >= 16 && bytesOut == 16+Pout->hdr.len){ // done sending
				deletePacket(Pout);
				Pout = nullptr;
			}
		}
	}else{

	}
	return true;
}

void ClientCommunicator::close(){
#ifdef _WIN32
	closesocket(sock);
#else
	::close(sock);
#endif
}

