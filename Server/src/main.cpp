/*
 * main.cpp
 *
 *  Created on: Apr 25, 2014
 *      Author: jeffrey
 */


#include "stdio.h"
#include <thread>         // std::this_thread::sleep_for
#include <chrono>
#include "SocketClient.h"

std::vector<SocketClient> C;

void sendToAll(const char *dta, unsigned int len){
	for(int i=0; i<C.size(); i++){
		if(C[i].isAuthenticated())
			C[i].sendPacket(dta, len);
	}
}

int main(int argc, char** argv)
{
	FractalGen gen(&sendToAll);

	int portno = 14860;

	std::cout << "localhost / port " << portno << "\n";
	std::cout << "Login Options: \n";
	std::cout << gen.getDB()->auth.dumpAuthDetails() << "\n\n";

#ifdef _WIN32
	WSADATA WsaDat;
    if (WSAStartup(MAKEWORD(2, 2), &WsaDat) != 0)
    {
		std::cerr << "WinSocket Initialization Failed\n";
		exit(0);
    } 
#endif

	struct sockaddr_in addr;
#ifdef _WIN32
	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET) {
		std::cerr << "Socket Creation Failed\n";
		std::cerr << "  Error" << WSAGetLastError() << "\n";
		exit(0);
	}
#else
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		std::cerr << "Socket Creation Failed\n";
		exit(0);
	}
#endif

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portno);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		std::cerr << "Socket Binding Failed: " << errno << "\n";
		exit(0);
	}
#ifdef _WIN32
	u_long iMode = 1;
	int res = ioctlsocket(sock, FIONBIO, &iMode);
	if (res != NO_ERROR){
		std::cerr << "ioctlsocket [For Non-Blocking Mode] failed with error: " << res << "\n";
		exit(0);
	}
#else
	fcntl(sock, F_SETFL, O_NONBLOCK); // Non-Blocking Mode
#endif

    if(-1 == listen(sock, 30))
    {
    	std::cerr << "Socket Listen Failed: " << errno << "\n";
		exit(0);
    }

    std::cout << "Ready For Connections...\n\n";

    std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
    std::ostringstream strCout;
    std::cout.rdbuf( strCout.rdbuf() );

    // now main loop
    unsigned long iteration = 0;
    while(true){

    	sockaddr_in cli_addr;
    	
#ifdef _WIN32
    	int sz = sizeof(cli_addr);
		SOCKET newClient = accept(sock, (struct sockaddr *) &cli_addr, &sz);
		int err = WSAGetLastError();
    	if(newClient != INVALID_SOCKET){
    		// spawn client
			u_long iMode = 1;
			int res = ioctlsocket(newClient, FIONBIO, &iMode);
			if (res != NO_ERROR){
				std::cerr << "ioctlsocket [For Non-Blocking Mode] failed with error: " << res << "\n";
				exit(0);
			}
			SocketClient cli(newClient, cli_addr, &gen);
    		C.push_back(cli);
    		std::cout << "Client Connected\n";
    	}else if(err != WSAEWOULDBLOCK){
    		std::cerr << "Socket Accept Failed: " << err << "\n";
			exit(0);
    	}
#else
    	socklen_t sz = sizeof(cli_addr);
    	int newClient = accept(sock, (struct sockaddr *) &cli_addr, &sz);
    	if(newClient > 0){
    		// spawn client
			int flags = fcntl(newClient, F_GETFL, 0);
    		fcntl(newClient, F_SETFL, flags | O_NONBLOCK);
			SocketClient cli(newClient, cli_addr, &gen);
    		C.push_back(cli);
    		std::cout << "Client Connected\n";
    	}else if(errno != EWOULDBLOCK && errno != EAGAIN){
    		std::cerr << "Socket Accept Failed: " << errno << "\n";
			exit(0);
    	}
#endif
    		

    	// check if any clients have something to say    \o/
    	auto it = C.begin();
    	while(it != C.end()){
    		if(!(*it).update())
    			C.erase(it); // client has disconnected, au revoir
    		else
    			it ++;
    	}

    	if(iteration % 50 == 0){
    		for(int i=0; i<C.size(); i++){
    			C[i].sendStatusUpdate();
    		}
    	}

    	gen.update();
    	std::this_thread::sleep_for(std::chrono::milliseconds(100));
    	iteration ++;


    }

    std::cout.rdbuf( oldCoutStreamBuf );
#ifdef _WIN32
	closesocket(sock);
#else
	close(sock);
#endif



#ifdef _WIN32
	WSACleanup();
#endif

	return 0;
}

