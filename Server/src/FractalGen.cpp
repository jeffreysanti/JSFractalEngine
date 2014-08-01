/*
 * FractalGen.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#include "FractalGen.h"
#include "FractalMultibrot.h"
#include "FractalJulia.h"

std::string FractalGen::saveDir = "./renders";

int dummy(){
	return 0;
}

FractalGen::FractalGen(void (*ret)(const char *dta, unsigned int len)) : db("./") {
	saveDir = "./renders";
	std::string cmd = "mkdir "+saveDir;
	system(cmd.c_str());
	maxThreads = 2;
	sendToAll = ret;
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
	FractalMeta m = gen->getDB()->getFractal(id);
	m.status = FDBS_CANCEL;
	gen->getDB()->updateFractal(m);
	char *dta = NULL;
	int len=0;
	gen->fillMDUDRequest(id, &dta, len);
	if(len > 0){
		(*gen->sendToAll)(dta, len);
		delete [] dta;
	}
}

// Removes from queues
bool FractalGen::cancelJob(unsigned int id, int uid, bool admin)
{
	for(int i=0; i<R.size(); i++){
		if(R[i]->jid == id && R[i]->fract != NULL && (R[i]->uid == uid || admin)){
			R[i]->fract->doCancel();
			canceled(id, this);
			return true;
		}
	}

	for(auto it=JQManual.begin(); it!=JQManual.end(); it++){
		if((*it)->getID() == id  && ((*it)->getUID() == uid || admin)){
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
		if((*it)->getID() == id  && ((*it)->getUID() == uid || admin)){
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
	FractalMeta m = db.getFractal(p->getID());
	m.status = FDBS_RENDERING;
	db.updateFractal(m);
	char *dta = NULL;
	int len=0;
	fillMDUDRequest(p->getID(), &dta, len);
	if(len > 0){
		(*sendToAll)(dta, len);
		delete [] dta;
	}

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

int FractalGen::postJob(Paramaters *p, int uid, bool daemon)
{
	FractalMeta m;
	m.userID = uid;
	m.jobID = db.getNextID();
	m.status = FDBS_QUEUED;
	m.author = p->getValue("author", "Unknown");
	m.name = p->getValue("title", "Untitled");
	db.insertFractal(m);

	p->setID(m.jobID);
	p->setUID(m.userID);
	if(!daemon){
		p->setValue("QUEUE", "MANUAL");
		JQManual.push_back(p);
	}else{
		p->setValue("QUEUE", "AUTO");
		JQ.push_back(p);
	}
	p->writeToFile(concat(saveDir+"/", p->getID())+".job");
	return m.jobID;
}

int FractalGen::postJob(std::string fl, int uid, bool daemon)
{
	Paramaters *p = new Paramaters();
	if(!p->loadFromFile(fl))
		return -1;
	return postJob(p, uid, daemon);
}

void FractalGen::update()
{
	auto it = R.begin();
	while(it != R.end()){
		if(isBusy(*it)){
			it ++;
			continue;
		}
		FractalMeta m = db.getFractal((*it)->jid);
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
		db.updateFractal(m);
		char *dta = NULL;
		int len=0;
		fillMDUDRequest((*it)->jid, &dta, len);
		if(len > 0){
			(*sendToAll)(dta, len);
			delete [] dta;
		}
		std::cout << "REMOVING(" << (*it)->jid << ")\n";
		deleteRunningJob(*it);
		delete (*it);
		R.erase(it);
	}

	// If we are not running max jobs, run one ;)
	if(R.size() < maxThreads){
		if(JQ.size() > 0 || JQManual.size() > 0){
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
}

int FractalGen::remainingJobs()
{
	return R.size() + JQ.size() + JQManual.size();
}
int FractalGen::jobsInManualQueue()
{
	return JQManual.size();
}
int FractalGen::jobsInAutoQueue()
{
	return JQ.size();
}
std::vector<unsigned int> FractalGen::currentJobIDs()
{
	std::vector<unsigned int> v;
	for(int i=0; i<R.size(); i++){
		v.push_back(R[i]->jid);
	}
	return v;
}
int FractalGen::currentJobExecutionTime(unsigned int jid)
{
	for(int i=0; i<R.size(); i++){
		if(R[i]->jid == jid)
			return time(NULL) - R[i]->timeStart;
	}
	return -1;
}

int FractalGen::ownerOfCurrentJob(unsigned int jid)
{
	for(int i=0; i<R.size(); i++){
		if(R[i]->jid == jid)
			return R[i]->uid;
	}
	return -1;
}

std::string FractalGen::getJobQueue(unsigned int uid)
{
	std::string ret = "";
	for(auto it=JQManual.begin(); it!=JQManual.end(); it++){
		if((*it)->getUID() == uid)
			ret = concat(ret+",U",(*it)->getID());
		else
			ret = concat(ret+",",(*it)->getID());
	}
	if(ret.length() > 0)
		ret = ret.substr(1); // remove first comma
	ret += "|";
	for(auto it=JQ.begin(); it!=JQ.end(); it++){
		if((*it)->getUID() == uid)
			ret = concat(ret+"U",(*it)->getID()) + ",";
		else
			ret = concat(ret,(*it)->getID()) + ",";
	}
	if(ret.at(ret.length()-1) == ',')
		ret = ret.substr(0, ret.length()-1); // remove last comma
	return ret;
}

User FractalGen::authenticateUser(std::string user, std::string pass)
{
	User u = db.auth.getByName(user);
	if(u.pass == pass)
		return u;
	return User(); // authentication failed -> return empty user
}

DBSystem *FractalGen::getDB(){
	return &db;
}

void FractalGen::fillMDUDRequest(int jid, char **sz, int &len)
{
	try{
		int iTmp = 0;

		FractalMeta fract = db.getFractal(jid);
		if(fract.jobID < 0){
			*sz = NULL;
			len = 0;
			return;
		}

		// layout: [MDUD][jid:4][uid:4][titleLen:4][title][authorLen:4][author][status:4][imgDtaLen:4][imgDta]
		// size: 28+|title| + |author| + |imgData|

		// open image
		std::string imgPath = concat(getSaveDir()+"/", jid) + ".png";
		FILE *fp = fopen(imgPath.c_str(), "rb");
		if(fp == NULL){ // no image
			len = 28 + fract.name.length() + fract.author.length();
			*sz = new char[len];
			memset(*sz + len - 4, 0, 4); // copy last four bytes as zero length of imgDta
		}else{
			fseek(fp, 0L, SEEK_END);
			int dtaSz = ftell(fp);
			fseek(fp, 0L, SEEK_SET);
			len = 28 + fract.name.length() + fract.author.length() + dtaSz;
			*sz = new char[len];

			iTmp = htonl(dtaSz);
			memcpy(*sz + len - 4 - dtaSz, &iTmp, 4); // copy last four bytes as zero length of imgDta
			char *ptr = *sz + len - dtaSz;
			for(int i=0; i<dtaSz; i++){
				char byte;
				fread(&byte, 1, 1, fp);
				*ptr = byte;
				ptr ++;
			}
			fclose(fp);
		}

		// now copy rest of data
		memcpy(*sz, "MDUD", 4);

		char *ptr = *sz + 4;

		iTmp =htonl(fract.jobID);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(fract.userID);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(fract.name.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, fract.name.c_str(), fract.name.length());
		ptr += fract.name.length();

		iTmp = htonl(fract.author.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, fract.author.c_str(), fract.author.length());
		ptr += fract.author.length();

		iTmp = htonl(fract.status);
		memcpy(ptr, &iTmp, 4);

	}catch(std::bad_alloc &e){
		std::cerr << "Out of memory\n";
		exit(0);
	}
}

void FractalGen::fillRCTXRequest(int jid, char **sz, int &len)
{
	try{
		int iTmp = 0;

		FractalMeta fract = db.getFractal(jid);
		if(fract.jobID < 0){
			*sz = NULL;
			len = 0;
			return;
		}

		// layout: [RCTX][jid:4][paramLen:4][params][poutLen:4][pOut][logLen:4][log][histLen:4][histo]
		// size: 24+|params| + |pOut| + |log| + |histo|

		std::string basePath = concat(getSaveDir()+"/", jid);

		// pOut
		Paramaters p;
		p.loadFromFile(basePath + ".info");
		std::string paramsOut = p.asString();

		// log
		std::fstream fp;
		std::string log = "";
		fp.open(std::string(basePath + ".log").c_str(), std::ios::in);
		if(fp.is_open()){
			std::string line;
			while(getline(fp,line))
			{
				log += line + "\n";
			}
		}

		// params----
		p.loadFromFile(basePath + ".job");
		std::string params = p.asString();

		ParamaterSchema schemBase("SCHEMA_BASE");
		ParamaterSchema schemMandl("SCHEMA_MULTIBROT");

		// histo
		int dtaSkip = schemBase.getInt(p, "imgWidth") * schemBase.getInt(p, "imgHeight") * 4;
		int iters = schemMandl.getInt(p, "iters");
		unsigned int *histogram = NULL;
		FILE *fp2 = fopen(std::string(basePath + ".algo").c_str(), "rb");
		if(iters > 0 && fp2 != NULL){
			fseek(fp2, dtaSkip, SEEK_SET);
			histogram = new unsigned int[iters];
			for(int i=0; i<iters; i++){
				int tmp;
				fread(&tmp, sizeof(unsigned int), 1, fp2);
				histogram[i] = htonl(tmp);
			}
		}
		if(fp2 != NULL)
			fclose(fp2);

		len = 24 + paramsOut.length() + params.length() + log.length();
		if(histogram != NULL)
			len += iters * 4;

		*sz = new char[len];
		memcpy(*sz, "RCTX", 4);

		char *ptr = *sz + 4;

		iTmp =htonl(jid);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(params.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, params.c_str(), params.length());
		ptr += params.length();

		iTmp =htonl(paramsOut.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, paramsOut.c_str(), paramsOut.length());
		ptr += paramsOut.length();

		iTmp =htonl(log.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, log.c_str(), log.length());
		ptr += log.length();

		if(histogram == NULL){
			iTmp = 0;
			memcpy(ptr, &iTmp, 4);
			ptr += 4;
		}else{
			iTmp = htonl(iters * 4);
			memcpy(ptr, &iTmp, 4);
			ptr += 4;

			memcpy(ptr, histogram, iters*4);
			ptr += iters * 4;
		}
	}catch(std::bad_alloc &e){
		std::cerr << "Out of memory\n";
		exit(0);
	}
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

		if(type == "multibrot"){
			r->fract = new FractalMultibrot(r->params->getID(), r->params, pOut, img);
			r->fract->processParams();
			if(!r->fract->isOkay()){
				std::cerr << r->fract->getErrors();
				r->timeStart = TIMESTART_COMPLETE;
				return 0;
			}
			r->fract->render(timeOut);
		}else if(type == "julia"){
			r->fract = new FractalJulia(r->params->getID(), r->params, pOut, img);
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



