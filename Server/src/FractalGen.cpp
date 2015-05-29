/*
 * FractalGen.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#include "FractalGen.h"
#include "FractalMandleJulia.h"

std::string FractalGen::saveDir = "";

int dummy(){
	return 0;
}

FractalGen::FractalGen() {
	saveDir = DirectoryManager::getSingleton()->getRootDirectory()+"renders";


#ifdef _WIN32
	CreateDirectoryA(saveDir.c_str(), NULL);
#else
	mkdir(saveDir.c_str(), 0700);
#endif

	maxThreads = 2;
	if(DirectoryManager::getSingleton()->getConfigRoot().isMember("maxthreads") &&
			DirectoryManager::getSingleton()->getConfigRoot()["maxthreads"].isInt()){
		maxThreads = DirectoryManager::getSingleton()->getConfigRoot()["maxthreads"].asInt();
		std::cout << "Using " << maxThreads << " Rendering Threads.\n";
	}
}

FractalGen::~FractalGen() {
	for(int i=0; i<R.size(); i++){
		deleteRunningJob(R[i]);
		delete R[i];
	}
	R.clear();
}

void FractalGen::deleteRunningJob(RenderingJob *job)
{
	SAFE_DELETE(job->fract);
	job->params = NULL;
}

std::string FractalGen::getSaveDir()
{
	return saveDir;
}


inline void canceled(unsigned int id, FractalGen *gen){
	FractalMeta m = DBManager::getSingleton()->getFractal(id);
	m.status = FDBS_CANCEL;
	DBManager::getSingleton()->updateFractal(m);
	InfoExchange exh;
	exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
	exh.uiParam1 = m.jobID;
	FractalGenTrackManager::getSingleton()->postExchange(exh);
}


// Removes from queues
bool FractalGen::cancelJob(unsigned int id)
{
	// remove from animation queue
	cancelAnimation(id);

	for(int i=0; i<R.size(); i++){
		if(R[i]->jid == id && R[i]->fract != NULL){
			R[i]->fract->doCancel();
			canceled(id, this);
			return true;
		}
	}

	for(auto it=JQManual.begin(); it!=JQManual.end(); it++){
		if((*it)->getJson()["internal"]["id"].asUInt() == id){
			ParamsFileNotSchema *pOut = new ParamsFileNotSchema();
			Fractal *fTmp = new Fractal((*it)->getJson()["internal"]["id"].asUInt(), (*it), pOut);
			fTmp->doCancel();
			fTmp->updateStatus("JobQueueCanceling", 100);
			pOut->saveToFile(concat(saveDir+"/", (*it)->getJson()["internal"]["id"].asUInt())+".info");
			delete fTmp;
			JQManual.erase(it);
			canceled(id, this);
			return true;
		}
	}
	for(auto it=JQ.begin(); it!=JQ.end(); it++){
		if((*it)->getJson()["internal"]["id"].asUInt() == id){
			ParamsFileNotSchema *pOut = new ParamsFileNotSchema();
			Fractal *fTmp = new Fractal((*it)->getJson()["internal"]["id"].asUInt(), (*it), pOut);
			fTmp->doCancel();
			fTmp->updateStatus("JobQueueCanceling", 100);
			pOut->saveToFile(concat(saveDir+"/", (*it)->getJson()["internal"]["id"].asUInt())+".info");
			delete fTmp;
			JQ.erase(it);
			canceled(id, this);
			return true;
		}
	}
	return false;
}

void FractalGen::runThread(ParamsFile *p)
{
	// message that rendering stopped
	FractalMeta m = DBManager::getSingleton()->getFractal(p->getJson()["internal"]["id"].asInt());
	m.status = FDBS_RENDERING;
	DBManager::getSingleton()->updateFractal(m);
	char *dta = NULL;
	int len=0;

	InfoExchange exh;
	exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
	exh.uiParam1 = m.jobID;
	FractalGenTrackManager::getSingleton()->postExchange(exh);

	std::cout << "runThread(" << p->getJson()["internal"]["id"].asInt() << ")\n";
	RenderingJob *r = new RenderingJob;

	if(p->getJson()["internal"].isMember("thisframe") &&
			p->getJson()["internal"]["thisframe"].isInt()){
		r->frameID = p->getJson()["internal"]["thisframe"].asInt();
	}else{
		r->frameID = 0;
	}

	r->jid = p->getJson()["internal"]["id"].asInt();
	r->params = p;
	r->timeStart = time(NULL);
	r->fract = NULL;
	r->genPtr = this;
	r->gen = std::thread(&runGen, r);
	r->gen.detach();
	R.push_back(r);
}

bool FractalGen::isBusy(RenderingJob *job)
{
	if(job->timeStart != TIMESTART_COMPLETE)
		return true;

	return false;
}

int FractalGen::postJob(FractalContainer f)
{
	f.m.status = FDBS_QUEUED;
	f.m.author = "Unknown";
	f.m.name = "Untitled";
	if(f.p->getJson().isObject() && f.p->getJson().isMember("basic") && f.p->getJson()["basic"].isObject()){
		if(f.p->getJson()["basic"].isMember("author"))
			f.m.author = f.p->getJson()["basic"]["author"].asString();
		if(f.p->getJson()["basic"].isMember("title"))
			f.m.name = f.p->getJson()["basic"]["title"].asString();
	}
	DBManager::getSingleton()->updateFractal(f.m);

	f.p->getJson()["internal"]["id"] = f.m.jobID;
	f.p->getJson()["internal"]["uid"] = f.m.userID;
	f.p->clearFrameData(); // frames cannot be submitted this way
	if(f.m.manualQueue){
		f.p->getJson()["internal"]["queue"] = "MANUAL";
		JQManual.push_back(f.p);
	}else{
		f.p->getJson()["internal"]["queue"] = "AUTO";
		JQ.push_back(f.p);
	}
	f.p->saveToFile(concat(saveDir+"/", f.m.jobID)+".job");
	return f.m.jobID;
}

void FractalGen::update()
{
	bool changeOccured = false;
	auto it = R.begin();
	while(it != R.end()){
		if(isBusy(*it)){
			it ++;
			continue;
		}
		changeOccured = true;

		if((*it)->frameID == 0){ // not an animation
			FractalMeta m = DBManager::getSingleton()->getFractal((*it)->jid);
			if((*it)->fract != NULL){
				if((*it)->fract->getState() == FS_TIMEOUT)
					m.status = FDBS_TIMEOUT;
				else if((*it)->fract->getState() == FS_CANCEL)
					m.status = FDBS_CANCEL;
				else if((*it)->fract->getState() == FS_DONE)
					m.status = FDBS_COMPLETE;
				else
					m.status = FDBS_ERR;
			}else{
				m.status = FDBS_ERR;
			}
			DBManager::getSingleton()->updateFractal(m);

			InfoExchange exh;
			exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
			exh.uiParam1 = m.jobID;
			FractalGenTrackManager::getSingleton()->postExchange(exh);
		}else{
			FractalDBState stat = FDBS_COMPLETE;
			if((*it)->fract != NULL){
				if((*it)->fract->getState() == FS_TIMEOUT)
					stat = FDBS_TIMEOUT;
				else if((*it)->fract->getState() == FS_CANCEL)
					stat = FDBS_CANCEL;
				else if((*it)->fract->getState() == FS_DONE)
					stat = FDBS_COMPLETE;  // OK
				else
					stat = FDBS_ERR;
			}else{
				stat = FDBS_ERR;
			}
			if(stat != FDBS_COMPLETE){
				// cancel job -> it failed
				cancelJob((*it)->jid);
			}
		}

		std::cout << "REMOVING(" << (*it)->jid << ")\n";
		deleteRunningJob(*it);
		delete (*it);
		R.erase(it);
	}

	addAnimationFramesToRenderQueue(maxThreads - R.size());

	// If we are not running max jobs, run one ;)
	if(R.size() < maxThreads){
		if(JQ.size() > 0 || JQManual.size() > 0){
			changeOccured = true;
			if(JQManual.size() > 0){
				ParamsFile *p = JQManual.front();
				JQManual.pop_front();
				runThread(p);
			}else{
				ParamsFile *p = JQ.front();
				JQ.pop_front();
				runThread(p);
			}
		}
	}
	if(changeOccured){ // need to make sure clients know if this
		sendUpdateToTracker();
	}

	// now we need to add any submitted jobs to queues
	std::vector<FractalContainer> newJobs = DBManager::getSingleton()->fetchSubmittedJobs();
	for(auto it=newJobs.begin(); it!=newJobs.end(); it++){
		postJob(*it);
	}

	// now check if any jobs need to be modified
	InfoExchange exh = FractalGenTrackManager::getSingleton()->getExchange(EXT_AFFECT_RENDERING_FRACTAL);
	if(exh.type == EXT_AFFECT_RENDERING_FRACTAL){ // exists
		int jid = exh.uiParam1;
		if(exh.artType == ART_CANCEL){
			cancelJob(jid);
		}else if(exh.artType == ART_DELETE){
			FractalMeta meta = DBManager::getSingleton()->getFractal(jid);
			if(meta.jobID == jid){
				DBManager::getSingleton()->deleteFractal(meta);
				InfoExchange exh;
				exh.type = EXT_REPORT_DELETED;
				exh.uiParam1 = meta.jobID;
				FractalGenTrackManager::getSingleton()->postExchange(exh);
			}
		}else if(exh.artType == ART_CHANGE_NAME || exh.artType == ART_CHANGE_AUTHOR){ // some kind of simple update
			FractalMeta meta = DBManager::getSingleton()->getFractal(jid);
			if(meta.jobID == jid){
				if(exh.artType == ART_CHANGE_NAME && exh.sParam3.length() > 0){
					meta.name = exh.sParam3;
				}if(exh.artType == ART_CHANGE_AUTHOR && exh.sParam3.length() > 0){
					meta.author = exh.sParam3;
				}
				DBManager::getSingleton()->updateFractal(meta);
				InfoExchange exh;
				exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
				exh.uiParam1 = meta.jobID;
				FractalGenTrackManager::getSingleton()->postExchange(exh);
			}
		}
	}

}

void FractalGen::sendUpdateToTracker()
{
	JobStatus j;
	std::vector<JobStatus> jobs;
	for(int i=0; i<R.size(); i++){
		j.jid = R[i]->jid;
		j.owner = R[i]->uid;
		j.timeStart = R[i]->timeStart;
		jobs.push_back(j);
	}
	FractalGenTrackManager::getSingleton()->postCurrentJobs(jobs);

	jobs.clear();

	for(auto it=JQManual.begin(); it!=JQManual.end(); it++){
		j.jid = (*it)->getJson()["internal"]["id"].asUInt();
		j.owner = (*it)->getJson()["internal"]["uid"].asUInt();
		j.type = 1;
		jobs.push_back(j);
	}
	for(auto it=JQ.begin(); it!=JQ.end(); it++){
		j.jid = (*it)->getJson()["internal"]["id"].asUInt();
		j.owner = (*it)->getJson()["internal"]["uid"].asUInt();
		j.type = 2;
		jobs.push_back(j);
	}

	FractalGenTrackManager::getSingleton()->postQueuedJobs(jobs);
}


void FractalGen::addAnimationToQueue(Animation anim){
	animLock.lock();
	FractalMeta m = DBManager::getSingleton()->getFractal(anim.baseID);
	if(m.manualQueue)
		anim.manualQueue = true;
	else
		anim.manualQueue = false;
	A.push_back(anim);
	animLock.unlock();
}

void FractalGen::addAnimationFramesToRenderQueue(int count){
	unsigned long now = time(NULL);
	for(int i=0; i<A.size(); i++) {
		if(A[i].timeMustStop == 0 || A[i].timeMustStop > now){
			continue;
		}
		FractalLogger::getSingleton()->write(A[i].baseID, "TIMED OUT!\n");
		cancelJob(A[i].baseID);
		break; // only cancel one this pass to avoid concurrent access
	}

	animLock.lock();
	int a = 0;
	while(count > 0 && a < A.size()){
		int jid = A[a].baseID;
		while(count > 0 && !A[a].frameQueue.empty()){
			std::string frameFile = A[a].frameQueue.front();
			A[a].frameQueue.pop_front();

			ParamsFile *p = new ParamsFile(frameFile, true);
			if(A[a].manualQueue){
				JQManual.push_back(p);
			}else{
				JQ.push_back(p);
			}
			A[a].framesRendering.insert(p->getFrameData());
			count--;
		}
		a++;
	}
	// now check for animation progress
	for(auto itA = A.begin(); itA != A.end(); ) {
		int jid = (*itA).baseID;

		// process frames
		for(auto itF = (*itA).framesRendering.begin(); itF != (*itA).framesRendering.end(); ) {
			int fno = *itF;
			bool stillbusy = false;

			if((*itA).manualQueue){
				for(ParamsFile *f : JQManual){
					if(f->getID() == jid && f->getFrameData() == fno){
						stillbusy = true;
						break;
					}
				}
			}else{
				for(ParamsFile *f : JQ){
					if(f->getID() == jid && f->getFrameData() == fno){
						stillbusy = true;
						break;
					}
				}
			}

			if(stillbusy){
				++itF;
				continue;
			}
			for(RenderingJob *r : R){
				if(r->jid == jid && r->frameID == fno){
					stillbusy = true;
					break;
				}
			}

			if(stillbusy){
				++itF;
				continue;
			}
			// no longer rendering this frame
			itF = (*itA).framesRendering.erase(itF);


			double percent = 1.0 - (double)((*itA).frameQueue.size() +
					(*itA).framesRendering.size()) / (double)(*itA).frames;
			percent *= 100.0;
			FractalLogger::getSingleton()->write((*itA).baseID,
					concat("Frame Completed: ", percent) + "% complete\n");
		}

		// check if animation rendering complete
		if((*itA).frameQueue.empty() && (*itA).framesRendering.empty()){
			FractalMeta m = DBManager::getSingleton()->getFractal((*itA).baseID);
			m.status = FDBS_COMPLETE;
			DBManager::getSingleton()->updateFractal(m);

			InfoExchange exh;
			exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
			exh.uiParam1 = m.jobID;
			FractalGenTrackManager::getSingleton()->postExchange(exh);

			sendUpdateToTracker();

			itA = A.erase(itA);
		}else{
			++itA;
		}
	}
	animLock.unlock();
}

void FractalGen::cancelAnimation(int jid)
{
	animLock.lock();
	for(auto itA = A.begin(); itA != A.end(); ) {
		if((*itA).baseID != jid){
			++itA;
			continue;
		}
		itA = A.erase(itA);
	}
	animLock.unlock();
}


int runGen(RenderingJob *r)
{
	try{
		ParamsFileNotSchema *pOut = new ParamsFileNotSchema();

		std::string err = "";
		if(!r->params->validate(err)){
			Fractal *fTmp = new Fractal(r->params->getJson()["internal"]["id"].asInt(), r->params, pOut);
			fTmp->err(err);
			std::cerr << err;
			delete fTmp;
			r->timeStart = TIMESTART_COMPLETE;
			return 0;
		}

		int timeOut = r->params->getJson()["basic"]["timeout"].asInt();

		std::string animated = r->params->getJson()["basic"]["anim"]["selected"].asString();
		if(animated == "yes"){
			AnimationBuilder builder(r->params, pOut, r->params->getJson()["internal"]["id"].asInt());
			Animation anim = builder.spawnJobs(err, timeOut);
			if(err != ""){
				Fractal *fTmp = new Fractal(r->params->getJson()["internal"]["id"].asInt(), r->params, pOut);
				fTmp->err(err);
				std::cerr << err;
				delete fTmp;
				r->timeStart = TIMESTART_COMPLETE;
				return 0;
			}
			r->genPtr->addAnimationToQueue(anim);
			r->frameID = 1;
		}

		std::string type = r->params->getJson()["basic"]["type"]["selected"].asString();


		if(type == "mandlejulia"){
			r->fract = new FractalMandleJulia(r->params->getJson()["internal"]["id"].asInt(), r->params, pOut);
			r->fract->processParams();
			if(!r->fract->isOkay()){
				std::cerr << r->fract->getErrors();
				r->timeStart = TIMESTART_COMPLETE;
				return 0;
			}
			r->fract->render(timeOut);
		}else{
			Fractal *fTmp = new Fractal(r->params->getJson()["internal"]["id"].asInt(), r->params, pOut);
			fTmp->err("Error: Unknown type parameter: " + type);
			r->timeStart = TIMESTART_COMPLETE;
			delete fTmp;
			return 0;
		}

		if(!r->fract->isEndedEarly()){
			r->fract->updateStatus("Complete!!!", 100);
			pOut->getJson()["Complete"] = "YES";
		}

		if(r->fract->isOkay()){
			pOut->saveToFile(concat(FractalGen::getSaveDir()+"/", r->params->getJson()["internal"]["id"].asInt())+".info");
		}
		r->timeStart = TIMESTART_COMPLETE;
	}catch (const std::exception &exc){
		std::cerr << "EXCEPTION:\n";
	    std::cerr << exc.what() << "\n";
	    r->timeStart = TIMESTART_COMPLETE;
	    return 0;
	}catch(...){
		std::cerr << "Unknown Exception Caught\n";
		r->timeStart = TIMESTART_COMPLETE;
		return 0;
	}
	return 0;
}



