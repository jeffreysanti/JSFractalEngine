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
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <errno.h>
#include "Fractal.h"
#include "DBManager.h"
#include "FractalGenTrackManager.h"
#include "AnimationBuilder.h"

int dummy();

class FractalGen;

#define TIMESTART_COMPLETE 0
struct RenderingJob{

	RenderingJob(){
		uid = -1;
		jid = 0;
		params = NULL;
		fract = NULL;
		timeStart = 1; // zero for completed
		success = false;
	}

	unsigned int jid;
	unsigned int frameID;

	std::thread gen;
	ParamsFile *params;
	Fractal *fract;
	unsigned long timeStart;
	int uid;
	bool success;
	FractalGen *genPtr;
};


class FractalGen {
public:
	FractalGen();
	virtual ~FractalGen();

	static std::string getSaveDir();

	int postJob(FractalContainer f);
	void addAnimationToQueue(Animation anim);

	void update();



private:
	static std::string saveDir;

	void runThread(ParamsFile *p);
	bool isBusy(RenderingJob *job);

	void deleteRunningJob(RenderingJob *job);

	bool cancelJob(unsigned int id);

	unsigned short maxThreads;

	void sendUpdateToTracker();


	std::vector<RenderingJob*> R;

	// job stack
	std::list<ParamsFile*> JQ;
	std::list<ParamsFile*> JQManual;


	std::vector<Animation> A;
	std::mutex animLock;
	void addAnimationFramesToRenderQueue(int count);
	void cancelAnimation(int jid);
};

int runGen(RenderingJob *r);



#endif /* FRACTALGEN_H_ */
