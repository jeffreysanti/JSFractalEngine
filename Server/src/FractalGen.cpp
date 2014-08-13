/*
 * FractalGen.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#include "FractalGen.h"
#include "FractalMandleJulia.h"

std::string FractalGen::saveDir = "./renders";

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
	char *dta = nullptr;
	int len=0;
	InfoExchange exh;
	exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
	exh.uiParam1 = m.jobID;
	FractalGenTrackManager::getSingleton()->postExchange(exh);
}


// Removes from queues
bool FractalGen::cancelJob(unsigned int id)
{
	for(int i=0; i<R.size(); i++){
		if(R[i]->jid == id && R[i]->fract != NULL){
			R[i]->fract->doCancel();
			canceled(id, this);
			return true;
		}
	}

	for(auto it=JQManual.begin(); it!=JQManual.end(); it++){
		if((*it)->getID() == id){
			Paramaters *pOut = new Paramaters();
			Fractal *fTmp = new Fractal((*it)->getID(), (*it), pOut, NULL);
			fTmp->doCancel();
			fTmp->updateStatus("JobQueueCanceling", 100);
			pOut->writeToFile(concat(saveDir+"/", (*it)->getID())+".info");
			delete fTmp;
			JQManual.erase(it);
			canceled(id, this);
			return true;
		}
	}
	for(auto it=JQ.begin(); it!=JQ.end(); it++){
		if((*it)->getID() == id){
			Paramaters *pOut = new Paramaters();
			Fractal *fTmp = new Fractal((*it)->getID(), (*it), pOut, NULL);
			fTmp->doCancel();
			fTmp->updateStatus("JobQueueCanceling", 100);
			pOut->writeToFile(concat(saveDir+"/", (*it)->getID())+".info");
			delete fTmp;
			JQ.erase(it);
			canceled(id, this);
			return true;
		}
	}
	return false;
}

void FractalGen::runThread(Paramaters *p)
{
	// message that rendering stopped
	FractalMeta m = DBManager::getSingleton()->getFractal(p->getID());
	m.status = FDBS_RENDERING;
	DBManager::getSingleton()->updateFractal(m);
	char *dta = NULL;
	int len=0;

	InfoExchange exh;
	exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
	exh.uiParam1 = m.jobID;
	FractalGenTrackManager::getSingleton()->postExchange(exh);

	std::cout << "runThread(" << p->getID() << ")\n";
	RenderingJob *r = new RenderingJob;
	r->jid = p->getID();
	r->params = p;
	r->timeStart = time(NULL);
	r->fract = NULL;
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
	f.m.author = f.p->getValue("author", "Unknown");
	f.m.name = f.p->getValue("title", "Untitled");
	DBManager::getSingleton()->updateFractal(f.m);

	f.p->setID(f.m.jobID);
	f.p->setUID(f.m.userID);
	if(f.m.manualQueue){
		f.p->setValue("QUEUE", "MANUAL");
		JQManual.push_back(f.p);
	}else{
		f.p->setValue("QUEUE", "AUTO");
		JQ.push_back(f.p);
	}
	f.p->writeToFile(concat(saveDir+"/", f.p->getID())+".job");
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
		char *dta = NULL;
		int len=0;

		InfoExchange exh;
		exh.type = EXT_REPORT_FRACT_STATUS_CHANGE;
		exh.uiParam1 = m.jobID;
		FractalGenTrackManager::getSingleton()->postExchange(exh);


		std::cout << "REMOVING(" << (*it)->jid << ")\n";
		deleteRunningJob(*it);
		delete (*it);
		R.erase(it);
	}

	// If we are not running max jobs, run one ;)
	if(R.size() < maxThreads){
		if(JQ.size() > 0 || JQManual.size() > 0){
			changeOccured = true;
			if(JQManual.size() > 0){
				Paramaters *p = JQManual.front();
				JQManual.pop_front();
				runThread(p);
			}else{
				Paramaters *p = JQ.front();
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
		j.jid = (*it)->getID();
		j.owner = (*it)->getUID();
		j.type = 1;
		jobs.push_back(j);
	}
	for(auto it=JQ.begin(); it!=JQ.end(); it++){
		j.jid = (*it)->getID();
		j.owner = (*it)->getUID();
		j.type = 2;
		jobs.push_back(j);
	}

	FractalGenTrackManager::getSingleton()->postQueuedJobs(jobs);
}



int runGen(RenderingJob *r)
{
	try{

		//signal();

		ParamaterSchema schem("SCHEMA_BASE");

		std::string type = schem.getString(*r->params, "type");
		int width = schem.getInt(*r->params, "imgWidth");
		int height = schem.getInt(*r->params, "imgHeight");
		int timeOut = schem.getInt(*r->params, "timeOut");

		ImageWriter *img = new ImageWriter(width, height);
		Paramaters *pOut = new Paramaters();

		if(schem.isErr()){
			Fractal *fTmp = new Fractal(r->params->getID(), r->params, pOut, img);
			std::string errs = schem.clearErrors();
			fTmp->err(errs);
			std::cerr << errs;
			delete fTmp;
			r->timeStart = TIMESTART_COMPLETE;
			return 0;
		}

		if(type == "mj"){
			r->fract = new FractalMandleJulia(r->params->getID(), r->params, pOut, img);
			r->fract->processParams();
			if(!r->fract->isOkay()){
				std::cerr << r->fract->getErrors();
				r->timeStart = TIMESTART_COMPLETE;
				return 0;
			}
			r->fract->render(timeOut);
		}else{
			Fractal *fTmp = new Fractal(r->params->getID(), r->params, pOut, img);
			fTmp->err("Error: Unknown type parameter: " + type);
			r->timeStart = TIMESTART_COMPLETE;
			delete fTmp;
			return 0;
		}

		if(!r->fract->isEndedEarly()){
			r->fract->updateStatus("Complete!!!", 100);
			pOut->setValue("Complete", "YES");
		}

		if(r->fract->isOkay()){
			img->saveFile(concat(FractalGen::getSaveDir()+"/", r->params->getID())+".png");
			pOut->writeToFile(concat(FractalGen::getSaveDir()+"/", r->params->getID())+".info");
		}
		r->timeStart = TIMESTART_COMPLETE;
	}catch(...){
		std::cerr << "Unknown Exception Caught\n";
		r->timeStart = TIMESTART_COMPLETE;
		return 0;
	}
	return 0;
}



