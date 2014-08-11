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

void ClientCommunicator::sendNoSocketAuthorization()
{
	O.clear();
	User U = UserManager::getSingleton()->getNoSocketsAccount();
	addPacketToQueue("AUTH", concat("YESADMIN/",U.id), 0);
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
		if(bytesIn < sizeof(PacketHeader)){ //we are reading the header still
			int count = recv(sock, (char*)(Pin)+bytesIn, sizeof(PacketHeader)-bytesIn, 0);
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
			if(bytesIn == sizeof(PacketHeader)){ // header fully read
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
				}
			}
		}
		if(bytesIn >= sizeof(PacketHeader) && Pin->hdr.len > 0){ // read data section
			char* ptr = ((char*)Pin->data)+(bytesIn-sizeof(PacketHeader));
			int count = recv(sock, ptr, (Pin->hdr.len+sizeof(PacketHeader))-bytesIn, 0);
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
			if(bytesIn == Pin->hdr.len + sizeof(PacketHeader)){ // entire packet read -> reset reader
				R.push_back(Pin);
				Pin = new Packet;
				bytesIn = 0;
			}
		}else if(bytesIn == sizeof(PacketHeader) && Pin->hdr.len == 0){
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
			if(bytesOut < sizeof(PacketHeader)){ // send out header
				int count = send(sock, (char*)(Pout)+bytesOut, sizeof(PacketHeader)-bytesOut, 0);
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
			if(bytesOut == sizeof(PacketHeader)){ // change length back to host to compare later
				Pout->hdr.len = ntohl(Pout->hdr.len);
			}
			if(bytesOut >= sizeof(PacketHeader) && Pout->hdr.len > 0){ // write data section
				ssize_t count = send(sock, Pout->data+(bytesOut-sizeof(PacketHeader)), Pout->hdr.len-(bytesOut-sizeof(PacketHeader)), 0);
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
			if(bytesOut >= sizeof(PacketHeader) && bytesOut == sizeof(PacketHeader)+Pout->hdr.len){ // done sending
				deletePacket(Pout);
				Pout = nullptr;
			}
		}
	}else{
		// first check for any nesseary read operations [sin.lck exists]
		std::string inlock = DirectoryManager::getSingleton()->getRootDirectory()+"sin.lck";
		FILE *fp = fopen(inlock.c_str(),"r");
		if(fp != NULL){
			// read the packets
			std::string infl = DirectoryManager::getSingleton()->getRootDirectory()+"sin.sck";
			FILE *fp2 = fopen(infl.c_str(),"rb");
			if(fp2 != NULL){
				Packet *pkt = new Packet;

				// read the header [Only one packet allowed at a time]
				unsigned int count = fread(&pkt->hdr, 1, sizeof(PacketHeader), fp2);
				if(count == sizeof(PacketHeader)){ // okay to continue reading
					// fix byte ordering
					pkt->hdr.magic = ntohl(pkt->hdr.magic);
					pkt->hdr.len = ntohl(pkt->hdr.len);
					pkt->hdr.replyAddr = ntohl(pkt->hdr.replyAddr);

					// verify integrity
					if(pkt->hdr.magic == MAGIC && pkt->hdr.len <= MAXSZ){
						if(Pin->hdr.len > 0){
							assert(pkt->data == nullptr);
							pkt->data = new char[pkt->hdr.len];
							count = fread(pkt->data, 1, pkt->hdr.len, fp2);
							if(count == pkt->hdr.len){
								R.push_back(pkt);
							}else{
								deletePacket(pkt);
								pkt = nullptr;
							}
						}else{
							R.push_back(pkt);
						}
					}else{
						deletePacket(pkt);
						pkt = nullptr;
					}
				}else{
					pkt->data = nullptr; // in case this got corrupt nothing was allocated
					deletePacket(pkt);
					pkt = nullptr;
				}

				// clear the file
				fclose(fp2);
				fp2 = fopen(infl.c_str(),"wb");
				fclose(fp2);
			}

			// finally destroy the lock
			fclose(fp);
			remove(inlock.c_str());
		}

		// now send a packet if one exists
		if(O.size() > 0){
			Packet *pkt = O.front();
			O.pop_front();

			// make sure lock is gone
			std::string outlock = DirectoryManager::getSingleton()->getRootDirectory()+"sout.lck";
			FILE *fp = fopen(outlock.c_str(), "r");
			if(fp != NULL){
				fclose(fp);
			}else{
				std::string outfl = DirectoryManager::getSingleton()->getRootDirectory()+"sout.sck";
				fp = fopen(outfl.c_str(), "wb");

				// write header
				fwrite(&pkt->hdr, sizeof(PacketHeader), 1, fp);

				// now write data (if it exists)
				unsigned int len = ntohl(pkt->hdr.len);
				if(len > 0){
					fwrite(pkt->data, len, 1, fp);
				}

				fclose(fp);

				// write lock
				fp = fopen(outlock.c_str(), "wb");
				fclose(fp);
			}
		}


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

