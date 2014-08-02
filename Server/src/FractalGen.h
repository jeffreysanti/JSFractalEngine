/*
 * FractalGen.h
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#ifndef FRACTALGEN_H_
#define FRACTALGEN_H_

#include <ctime>
#include <cstring>
#include <stack>
#include <list>
#include <thread>
#include <csignal>

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

#include <errno.h>
#include "Fractal.h"
#include "DBSystem.h"

int dummy();

#define TIMESTART_COMPLETE 0
struct RenderingJob{

	RenderingJob(){
		uid = -1;
		jid = 0;
		params = NULL;
		fract = NULL;
		timeStart = 1; // zero for completed
	}

	unsigned int jid;

	std::thread gen;
	Paramaters *params;
	Fractal *fract;
	unsigned long timeStart;
	int uid;
	bool success;
};


class FractalGen {
public:
	FractalGen(void (*ret)(const char *dta, unsigned int len));
	virtual ~FractalGen();

	static std::string getSaveDir();

	bool cancelJob(unsigned int id, int uid, bool admin);

	int postJob(Paramaters *p, int uid, bool daemon);
	int postJob(std::string fl, int uid, bool daemon);

	void update();

	int remainingJobs();
	std::vector<unsigned int> currentJobIDs();
	int currentJobExecutionTime(unsigned int jid);
	int ownerOfCurrentJob(unsigned int jid);

	int jobsInManualQueue();
	int jobsInAutoQueue();

	std::string getJobQueue(unsigned int uid);

	User authenticateUser(std::string user, std::string pass);

	DBSystem *getDB();

	void fillMDUDRequest(int jid, char **sz, int &len);
	void fillRCTXRequest(int jid, char **sz, int &len);

	void (*sendToAll)(const char *dta, unsigned int len);

private:
	static std::string saveDir;

	void runThread(Paramaters *p);
	bool isBusy(RenderingJob *job);

	void deleteRunningJob(RenderingJob *job);

	unsigned short maxThreads;



	std::vector<RenderingJob*> R;

	// job stack
	std::list<Paramaters*> JQ;
	std::list<Paramaters*> JQManual;

	DBSystem db;
};

int runGen(RenderingJob *r);



#endif /* FRACTALGEN_H_ */
