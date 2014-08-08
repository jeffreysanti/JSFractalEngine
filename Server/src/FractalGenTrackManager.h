/*
 * FractalGenTrackManager.h
 *
 *  Created on: Aug 6, 2014
 *      Author: jeffrey
 */

#ifndef FRACTALGENTRACKMANAGER_H_
#define FRACTALGENTRACKMANAGER_H_

#include <mutex>
#include <vector>

enum EXCH_TYPE{
	EXT_NONE = 0,
	EXT_REPORT_FRACT_STATUS_CHANGE,
	EXT_AFFECT_RENDERING_FRACTAL
};

enum AFFECT_RENDERING_TYPE{
	ART_CANCEL = 1,
	ART_CHANGE_NAME,
	ART_CHANGE_AUTHOR
};


struct InfoExchange{

	InfoExchange(){
		type = EXT_NONE;
		uiParam1 = iParam2 = 0;
		artType = ART_CANCEL;
	}

	EXCH_TYPE type;
	unsigned int uiParam1;
	int iParam2;
	AFFECT_RENDERING_TYPE artType;
	std::string sParam3;
};

struct JobStatus{
	unsigned int jid;
	unsigned int owner;
	unsigned long timeStart;
	unsigned char type; // 1 for manual job, 2 for autoqueue
};


class FractalGenTrackManager {

public:

	void initialize();

	void postExchange(InfoExchange exc);
	InfoExchange getExchange(EXCH_TYPE type);

	void postCurrentJobs(std::vector<JobStatus>);
	std::vector<JobStatus> getCurrentJobs();

	void postQueuedJobs(std::vector<JobStatus>);
	std::vector<JobStatus> getQueuedJobs();


	static FractalGenTrackManager *getSingleton();

private:
	FractalGenTrackManager();
	virtual ~FractalGenTrackManager();

	std::vector<InfoExchange> E;

	std::vector<JobStatus> Jqueue;
	std::vector<JobStatus> Jcurrent;

	std::string jobs;

	std::mutex mtx;

	static FractalGenTrackManager singleton;
};

#endif /* FRACTALGENTRACKMANAGER_H_ */
