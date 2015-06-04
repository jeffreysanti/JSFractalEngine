/*
 * Fractal.cpp
 *
 *  Created on: May 24, 2014
 *      Author: jeffrey
 */

#include "Fractal.h"
#include "FractalGen.h"


Fractal::Fractal(unsigned int id, ParamsFile *params)
{
	fId = id;
	p = params;

	timeMustStop = 0;

	time_t now = time(NULL);
	char* dt = ctime(&now);

	if(p->getFrameData() == 0 || p->getFrameData() == 1){
		Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
		pOut["id"] = id;
		pOut["initTime"] = std::string(dt).substr(0, strlen(dt)-1); // remove newline char
		FractalLogger::getSingleton()->unlockOutParams(fId);
	}
	stageName = "";
	state = FS_CONSTRUCTED;
}

Fractal::~Fractal()
{
	time_t now = time(NULL);
	char* dt = ctime(&now);

	if(p->getFrameData() == 0){
		Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
		pOut["freeTime"] = std::string(dt).substr(0, strlen(dt)-1);
		FractalLogger::getSingleton()->unlockOutParams(fId, true);
	}

	SAFE_DELETE(p);
}

bool Fractal::isEndedEarly()
{
	return state == FS_CANCEL || state == FS_TIMEOUT || state == FS_ERR;
}

void Fractal::doCancel()
{
	state = FS_CANCEL;
}

// Returns True when system shall abort
bool Fractal::updateStatus(std::string stage, double percent)
{
	static unsigned long last = 0;
	static unsigned long lastStateTrans = 0;
	unsigned long now = time(NULL);

	if(state == FS_ERR){
		return true; // error occured
	}
	if(state == FS_CANCEL)
	{
		Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
		pOut["Canceled"] = "YES";
		FractalLogger::getSingleton()->unlockOutParams(fId);
		logMessage("Canceled!\n", true);
		return true;
	}
	if(state == FS_TIMEOUT || (timeMustStop != 0 && now > timeMustStop)){
		Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
		pOut["TimedOut"] = "YES";
		FractalLogger::getSingleton()->unlockOutParams(fId);
		logMessage("Timed Out!\n", true);
		state = FS_TIMEOUT;
		return true;
	}

	if(stage != stageName){
		if(stageName != ""){
			unsigned long len = now - lastStateTrans;
			std::string pName = "time"+stageName;

			Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
			if(pOut.isMember(pName)){
				len = len + pOut[pName].asLargestInt();
			}
			pOut[pName] = (int)len;
			FractalLogger::getSingleton()->unlockOutParams(fId);
		}
		stageName = stage;
		if(stageName != "done"){
			logMessage("Starting Stage: " + stageName + "\n", false);
		}
		lastStateTrans = now;
	}
	if(stageName == "done")
		return false;

	if(now - last > 2){
		logMessage("Stage: " + stage + concat(", ", percent) + "% complete\n", false);
		last = now;
	}
	return false;
}
bool Fractal::isOkay()
{
	return state != FS_ERR;
}
std::string Fractal::getErrors()
{
	return errs.str();
}

void Fractal::render(int maxTime)
{
	renderStart = time(NULL);
	if(maxTime > 0)
		timeMustStop = renderStart + maxTime;
	else
		timeMustStop = 0;
	state = FS_RENDERING;
}
void Fractal::postRender()
{
	if(state == FS_RENDERING){
		updateStatus("done", 100);
		unsigned long end = time(NULL);

		Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
		unsigned long tRend = end-renderStart;
		if(pOut.isMember("TOTALRENDERTIME")){
			tRend = tRend + pOut["TOTALRENDERTIME"].asLargestInt();
		}
		pOut["TOTALRENDERTIME"] = (int)tRend;
		if(p->getFrameData() == 0){
			pOut["complete"] = "YES";
		}
		FractalLogger::getSingleton()->unlockOutParams(fId, true);
		state = FS_DONE;
	}
}
void Fractal::processParams()
{
	state = FS_PARAMS_PROCESSED;
	processParamStart = time(NULL);
}

void Fractal::postProcessParams()
{
	unsigned long end = time(NULL);
	unsigned long len = end - processParamStart;
	Json::Value &pOut = FractalLogger::getSingleton()->outParams(fId);
	if(pOut.isMember("timeParamProcessing")){
		len = len + pOut["timeParamProcessing"].asLargestInt();
	}
	pOut["timeParamProcessing"] = (int)len;
	FractalLogger::getSingleton()->unlockOutParams(fId);
}

unsigned int Fractal::getId()
{
	return fId;
}

void Fractal::err(std::string error)
{
	state = FS_ERR;
	errs << error;
	logMessage(error, true);
}

FractalState Fractal::getState(){
	return state;
}

void Fractal::logMessage(std::string s, bool important)
{
	if(p->getFrameData() == 0 || important){
		FractalLogger::getSingleton()->write(fId, s);
	}else{
		// repress message
	}
}



