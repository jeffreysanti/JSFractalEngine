/*
 * Fractal.cpp
 *
 *  Created on: May 24, 2014
 *      Author: jeffrey
 */

#include "Fractal.h"
#include "FractalGen.h"


Fractal::Fractal(unsigned int id, ParamsFile *params, ParamsFileNotSchema *paramsOut)
{
	fId = id;
	p = params;
	pOut = paramsOut;

	timeMustStop = 0;

	time_t now = time(NULL);
	char* dt = ctime(&now);

	pOut->getJson()["id"] = id;
	pOut->getJson()["initTime"] = std::string(dt).substr(0, strlen(dt)-1); // remove newline char
	stageName = "";
	state = FS_CONSTRUCTED;
}

Fractal::~Fractal()
{
	time_t now = time(NULL);
	char* dt = ctime(&now);

	pOut->getJson()["freeTime"] = std::string(dt).substr(0, strlen(dt)-1);

	SAFE_DELETE(p);
	SAFE_DELETE(pOut);
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
		pOut->getJson()["Canceled"] = "YES";
		logMessage("Canceled!\n", true);
		return true;
	}
	if(state == FS_TIMEOUT || (timeMustStop != 0 && now > timeMustStop)){
		pOut->getJson()["TimedOut"] = "YES";
		logMessage("Timed Out!\n", true);
		state = FS_TIMEOUT;
		return true;
	}

	if(stage != stageName){
		if(stageName != ""){
			unsigned long len = now - lastStateTrans;
			pOut->getJson()["time"+stage] = (int)len;
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

		pOut->getJson()["timeRender"] = int(end-renderStart);
		pOut->getJson()["complete"] = "YES";
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
	pOut->getJson()["timeParamProcessing"] = int(end-processParamStart);
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



