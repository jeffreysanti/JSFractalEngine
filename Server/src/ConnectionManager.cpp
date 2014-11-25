/*
 * ConnectionManager.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#include "ConnectionManager.h"

ConnectionManager ConnectionManager::singleton;

ConnectionManager::ConnectionManager() {
	sock = 0;
	networkThread = nullptr;
	nosockets = false;
	maxConnections = 25;
	nextClientID=1;
}

ConnectionManager::~ConnectionManager() {
	if(networkThread != nullptr){
		delete networkThread;
	}

	for(int i=0; i<C.size(); i++)
		delete C[i];
	C.clear();

	if(!nosockets){
#ifdef _WIN32
		closesocket(sock);
		WSACleanup();
#else
		close(sock);
#endif
	}else{

	}
}

void ConnectionManager::initialize()
{
	if(networkThread != nullptr){
		std::cerr << "ConnectionManager::initialize called multiple times!\n";
		exit(EXIT_FAILURE);
		return;
	}

	// get settings from directory manager
	Json::Value &root = DirectoryManager::getSingleton()->getConfigRoot();
	if(!root.isMember("portno") || !root["portno"].isInt()){
		std::cerr << "Configuration 'portno' invalid or non-existent!\n";
		exit(EXIT_FAILURE);
		return;
	}
	int portno = root["portno"].asInt();

	if(root.isMember("nosockets") && root["nosockets"].isBool() && root["nosockets"].asBool()){
		std::cout << "Using nosockets method:: WARNING may be slow and only supports one local connection\n";
		nosockets = true;
		maxConnections = 1;
	}else{
		std::cout << "Creating TCP Connection on port " << portno << "\n";
		nosockets = false;
		if(!root.isMember("maxclients") || !root["maxclients"].isInt()){
			std::cerr << "Configuration 'maxclients' invalid or non-existent!\n";
			exit(EXIT_FAILURE);
			return;
		}
		maxConnections = root["maxclients"].asInt();
	}
	std::cout << "Server will support " << maxConnections << " client(s)\n";

	if(!nosockets){

		// Establish Connection info
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(portno);
		addr.sin_addr.s_addr = htonl(INADDR_ANY);

#ifdef _WIN32
		// WIN32 Socket Creation
		WSADATA WsaDat;
	    if(WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
	    {
	    	std::cerr << "WinSocket Initialization Failed\n";
	    	exit(EXIT_FAILURE);
	    }
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == INVALID_SOCKET) {
			std::cerr << "Socket Creation Failed\n";
			std::cerr << " Error" << WSAGetLastError() << "\n";
			exit(EXIT_FAILURE);
		}
		// bind socket address
		if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
			std::cerr << "Socket Binding Failed: " << WSAGetLastError() << "\n";
			exit(EXIT_FAILURE);
		}
		// Win32 Non-Block
		u_long iMode = 1;
		int res = ioctlsocket(sock, FIONBIO, &iMode);
		if (res != NO_ERROR){
			std::cerr << "ioctlsocket [For Non-Blocking Mode] failed with error: " << res << "\n";
			exit(EXIT_FAILURE);
		}
		// now enable socket listening
		if(0 != listen(sock, 30))
		{
			std::cerr << "Socket Listen Failed: " << WSAGetLastError() << "\n";
			exit(EXIT_FAILURE);
		}
#else
		// UNIX socket creation
		sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock == -1) {
			std::cerr << "Socket Creation Failed: " << errno << "\n";
			exit(EXIT_FAILURE);
		}
		// bind socket address
		if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
			std::cerr << "Socket Binding Failed: " << errno << "\n";
			exit(EXIT_FAILURE);
		}
		// UNIX non-block
		fcntl(sock, F_SETFL, O_NONBLOCK); // Non-Blocking Mode
		// now enable socket listening
		if(-1 == listen(sock, 30))
		{
			std::cerr << "Socket Listen Failed: " << errno << "\n";
			exit(EXIT_FAILURE);
		}
#endif

	}else{
		// Non-Socket Method
		// Clear all files used by connection
		remove(std::string(DirectoryManager::getSingleton()->getRootDirectory()+"sout.sck").c_str());
		remove(std::string(DirectoryManager::getSingleton()->getRootDirectory()+"sin.sck").c_str());
		remove(std::string(DirectoryManager::getSingleton()->getRootDirectory()+"sout.lck").c_str());
		remove(std::string(DirectoryManager::getSingleton()->getRootDirectory()+"sin.sck").c_str());

		/*
		 *   File Functions
		 *   sout.sck :: Any Messages to client are written to this file
		 *   sout.lck :: Server Writes Trigger This file to be written; client removes when read complete
		 *
		 *   sin.sck :: Any Messages to server are written to this file
		 *   sin.lck :: Client Writes Trigger This file to be written; server removes when read complete
		 */

		// initalize files
		std::ofstream outfile(DirectoryManager::getSingleton()->getRootDirectory()+"sout.sck");
		outfile.close();

		std::ofstream outfile2(DirectoryManager::getSingleton()->getRootDirectory()+"sin.sck");
		outfile.close();

		// Now create our only client
		ClientCommunicator *com = new ClientCommunicator(); // nosockets type default constructor
		Client *cli = new Client(com);
		C.push_back(cli);
	}



	networkThread = new std::thread(ConnectionManager::threadMain);
	networkThread->detach();
	//ConnectionManager::threadMain();
}

ConnectionManager *ConnectionManager::getSingleton(){
	return &singleton;
}




void ConnectionManager::threadMain()
{
	std::cout << "ConnectionManager Thread Started\n";
	long lastUpdate = time(NULL);
	while(true){

		// first remove any disconnected clients and update them
		auto it=getSingleton()->C.begin();
		while(it != getSingleton()->C.end()){
			if(!(*it)->updateClient()){
				delete *(it);
				getSingleton()->C.erase(it);
			}else
				++it;
		}

		// now check for new communications
		if(!getSingleton()->nosockets && getSingleton()->C.size() < getSingleton()->maxConnections){ // using sockets
			sockaddr_in cli_addr;

#ifdef _WIN32
			int sz = sizeof(cli_addr);
			SOCKET newClient = accept(getSingleton()->sock, (struct sockaddr *) &cli_addr, &sz);
			int err = WSAGetLastError();
			if(newClient != INVALID_SOCKET){
				// spawn client
				u_long iMode = 1;
				int res = ioctlsocket(newClient, FIONBIO, &iMode);
				if (res != NO_ERROR){
					std::cerr << "ioctlsocket [For Non-Blocking Mode] failed with error: " << res << "\n";
					exit(EXIT_FAILURE);
				}
				ClientCommunicator *com = new ClientCommunicator(newClient, getSingleton()->nextClientID);
								Client *cli = new Client(com);
								getSingleton()->C.push_back(cli);
				std::cout << "Client Connected [" << getSingleton()->nextClientID << "]\n";
				getSingleton()->nextClientID++;
			}else if(err != WSAEWOULDBLOCK){
				std::cerr << "Socket Accept Failed: " << err << "\n";
				exit(EXIT_FAILURE);
			}
#else
			socklen_t sz = sizeof(cli_addr);
			int newClient = accept(getSingleton()->sock, (struct sockaddr *) &cli_addr, &sz);
			if(newClient > 0){
				// spawn client
				int flags = fcntl(newClient, F_GETFL, 0);
				fcntl(newClient, F_SETFL, flags | O_NONBLOCK);
				ClientCommunicator *com = new ClientCommunicator(newClient, getSingleton()->nextClientID);
				Client *cli = new Client(com);
				getSingleton()->C.push_back(cli);
				std::cout << "Client Connected [" << getSingleton()->nextClientID << "]\n";
				getSingleton()->nextClientID ++;
			}else if(errno != EWOULDBLOCK && errno != EAGAIN){
				std::cerr << "Socket Accept Failed: " << errno << "\n";
				exit(0);
			}
#endif
		} // end check for new socket connections


		// now generate server update message
		if(time(NULL) - lastUpdate >= 5){
			lastUpdate = time(NULL);
			std::string update = "Rendering: ";
			std::vector<JobStatus> J = FractalGenTrackManager::getSingleton()->getCurrentJobs();
			if(J.size() == 0)
				update += "<None>";
			else
				update += concat("",J[0].jid);
			for(int i=1; i<J.size(); i++){
				update += concat(", ",J[i].jid);
			}
			update += concat(" | ", FractalGenTrackManager::getSingleton()->getQueuedJobs().size()) + std::string(" Job(s) Remain");
			update += concat(" ", time(NULL));

			for(int i=0; i<singleton.C.size(); i++){
				singleton.C[i]->writeServerMessage(update);
			}
		}

		// now check global updates to send
		InfoExchange exh = FractalGenTrackManager::getSingleton()->getExchange(EXT_REPORT_FRACT_STATUS_CHANGE);
		if(exh.type != EXT_NONE){
			int jid = exh.uiParam1;

			for(int i=0; i<singleton.C.size(); i++){
				singleton.C[i]->writeServerFractalUpdate(jid);
			}
		}
		exh = FractalGenTrackManager::getSingleton()->getExchange(EXT_REPORT_DELETED);
		if(exh.type != EXT_NONE){
			int jid = exh.uiParam1;

			for(int i=0; i<singleton.C.size(); i++){
				singleton.C[i]->writeServerFractalDeletion(jid);
			}
		}


		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

