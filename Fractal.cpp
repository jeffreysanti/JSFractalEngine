/*
 * Fractal.cpp
 *
 *  Created on: May 24, 2014
 *      Author: jeffrey
 */

#include "Fractal.h"
#include "FractalGen.h"


Fractal::Fractal(unsigned int id, Paramaters *params, Paramaters *paramsOut, ImageWriter *i)
{
	fId = id;
	p = params;
	pOut = paramsOut;
	img = i;
	flogFile.open(std::string(concat(FractalGen::getSaveDir()+"/",id) +".log").c_str(), std::ios::out);

	timeMustStop = 0;

	time_t now = time(NULL);
	char* dt = ctime(&now);

	pOut->setValue("ID", concat("",id));
	pOut->setValue("initTime", std::string(dt).substr(0, strlen(dt)-1)); // remove newline char
	stageName = "";
	state = FS_CONSTRUCTED;
}

Fractal::~Fractal()
{
	time_t now = time(NULL);
	char* dt = ctime(&now);

	pOut->setValue("freeTime", std::string(dt).substr(0, strlen(dt)-1));
	flogFile.flush();
	flogFile.close();

	SAFE_DELETE(p);
	SAFE_DELETE(pOut);
	SAFE_DELETE(img);
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
		pOut->setValue("Canceled", "YES");
		flogFile << "Canceled!\n";
		return true;
	}
	if(state == FS_TIMEOUT || (timeMustStop != 0 && now > timeMustStop)){
		pOut->setValue("TimedOut", "YES");
		flogFile << "Timed Out!\n";
		state = FS_TIMEOUT;
		return true;
	}

	if(stage != stageName){
		if(stageName != ""){
			unsigned long len = now - lastStateTrans;
			pOut->setValue("time"+stage, concat("", len));
		}
		stageName = stage;
		if(stageName != "done")
			flogFile << "Starting Stage: " << stageName << "\n";
		lastStateTrans = now;
	}
	if(stageName == "done")
		return false;

	if(now - last > 2){
		flogFile << "Stage: "<<stage<<", " << percent << "% complete\n";
		flogFile.flush();
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
		pOut->setValue("timeRender", concat("", (end-renderStart)));
		pOut->setValue("complete", "YES");
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
	p->writeToFile(FractalGen::getSaveDir() + concat("/", getId())+".job");
	unsigned long end = time(NULL);
	pOut->setValue("timeParamProcessing", concat("", (end-processParamStart)));
}

unsigned int Fractal::getId()
{
	return fId;
}

void Fractal::err(std::string error)
{
	state = FS_ERR;
	errs << error;
	flogFile << error;
}

FractalState Fractal::getState(){
	return state;
}



