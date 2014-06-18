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


	struct sockaddr_in addr;
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == -1) {
		std::cerr << "Socket Creation Failed\n";
		exit(0);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(portno);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
		std::cerr << "Socket Binding Failed: " << errno << "\n";
		exit(0);
	}
	fcntl(sock, F_SETFL, O_NONBLOCK); // Non-Blocking Mode

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
    	unsigned int sz = sizeof(cli_addr);
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
    close(sock);


	/*while(true){

		// verify whether any tasks are submitted or other requests
		int bytes = 1;
		while(bytes > 0){
			char message[128];
			memset(&message, 0, 128);

			struct sockaddr_un remote;
			sizeOfAddress = sizeof(remote);

			bytes = recvfrom (sock, message, 128, 0,(struct sockaddr *) &remote, &sizeOfAddress);
			if(bytes <= 0)
				break;

			// now strip the message of control characters
			std::string msg = message;
			while(msg.find('\n') != msg.npos)
				msg = msg.replace(msg.find('\n'), 1, "");
			while(msg.find('\r') != msg.npos)
							msg = msg.replace(msg.find('\r'), 1, "");

			//std::cout << message << "\n";

			if(msg.find(':') == msg.npos)
				continue;

			std::string replyAddr = msg.substr(0, msg.find(':'));
			msg = msg.substr(msg.find(':')+1);

			// create reply address
			remote.sun_family = AF_LOCAL;
			strncpy (remote.sun_path, replyAddr.c_str(), sizeof (remote.sun_path));
			remote.sun_path[sizeof (remote.sun_path) - 1] = '\0';
			sizeOfAddress = SUN_LEN (&remote);

			std::string toSend;
			if(msg == "jobcount"){
				toSend = concat("",gen.remainingJobs());
			}else if(msg == "curjobid"){
				std::string tmp = "";
				auto jobs = gen.currentJobIDs();
				for(int i=0; i<jobs.size(); i++)
					tmp += concat("", jobs[i]) + ",";
				toSend = tmp;
			}else if(msg == "manualqueuesize"){
				toSend = concat("",gen.jobsInManualQueue());
			}else if(msg == "autoqueuesize"){
				toSend = concat("",gen.jobsInAutoQueue());
			}else if(msg == "curjobexctime"){
				toSend = "NA";//concat("",gen.currentJobExecutionTime());
			}else if(msg == "fullqueue"){
				toSend = gen.getJobQueue();
			}else if(msg == "cancelcur"){
				//gen.cancelJob();
				toSend = "NA";
			}else if(msg.substr(0,7) == "cancel|"){ // cancel listed job
				int jobid = atoi(msg.substr(7).c_str());
				if(gen.cancelJob(jobid))
					toSend = "OK";
				else
					toSend="FALSE";
			}else if(msg.substr(0,6) == "postm|" && msg.length() > 6){
				int jobid = gen.postJob(msg.substr(6), true);
				toSend = concat("",jobid);
			}else if(msg.substr(0,6) == "posta|" && msg.length() > 6){
				int jobid = gen.postJob(msg.substr(6), false);
				toSend = concat("",jobid);
			}else{
				toSend = "???";
			}


			int bytes = sendto(sock, toSend.c_str(), toSend.length()+1, 0,(struct sockaddr *) &remote, sizeOfAddress);
			if(bytes < 0){
				std::cerr << "Error Sending: " << errno << "\n";
			}
		}*/

		/*if(gen.remainingJobs() < 10){
			Paramaters *p = new Paramaters();
			p->setValue("type", "multibrot");
			p->setValue("fileOut", "out.png");
			p->setValue("imgWidth", "1024");
			p->setValue("imgHeight", "768");
			p->setValue("centR", "-0.627102");
			p->setValue("centI", "-0.623716");
			p->setValue("sizeR", "0.0645052");
			p->setValue("timeOut", "0");
			gen.postJob(p);
		}*/

		//gen.update();
		//std::this_thread::sleep_for(std::chrono::milliseconds(100));
	//}

	return 0;
}

