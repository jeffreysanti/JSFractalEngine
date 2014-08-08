/*
 * FractalGenTrackManager.cpp
 *
 *  Created on: Aug 6, 2014
 *      Author: jeffrey
 */

#include "FractalGenTrackManager.h"

FractalGenTrackManager FractalGenTrackManager::singleton;

FractalGenTrackManager::FractalGenTrackManager() {

}

FractalGenTrackManager::~FractalGenTrackManager() {
}

FractalGenTrackManager *FractalGenTrackManager::getSingleton(){
	return &singleton;
}

void FractalGenTrackManager::initialize(){

}

void FractalGenTrackManager::postExchange(InfoExchange exc)
{
	mtx.lock();
	E.push_back(exc);
	mtx.unlock();
}

InfoExchange FractalGenTrackManager::getExchange(EXCH_TYPE type)
{
	InfoExchange exh;
	mtx.lock();
	for(auto it = E.begin(); it != E.end(); it++){
		if((*it).type == type){
			exh = (*it);
			E.erase(it);
			break;
		}
	}
	mtx.unlock();
	return exh;
}

void FractalGenTrackManager::postCurrentJobs(std::vector<JobStatus> jobs)
{
	mtx.lock();
	Jcurrent = jobs;
	mtx.unlock();
}

std::vector<JobStatus> FractalGenTrackManager::getCurrentJobs()
{
	std::vector<JobStatus> cpy;
	mtx.lock();
	cpy = Jcurrent;
	mtx.unlock();

	return cpy;
}

void FractalGenTrackManager::postQueuedJobs(std::vector<JobStatus> jobs)
{
	mtx.lock();
	Jqueue = jobs;
	mtx.unlock();
}

std::vector<JobStatus> FractalGenTrackManager::getQueuedJobs()
{
	std::vector<JobStatus> cpy;
	mtx.lock();
	cpy = Jqueue;
	mtx.unlock();

	return cpy;
}





