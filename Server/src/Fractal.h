/*
 * Fractal.h
 *
 *  Created on: May 24, 2014
 *      Author: jeffrey
 */

#ifndef FRACTAL_H_
#define FRACTAL_H_


#include <vector>
#include <iostream>
#include <ctime>
#include <cstring>
#include <future>
#include <stack>
#include <list>
#include <sstream>
#include "ParamsFile.h"
#include "ImageWriter.h"

#define SAFE_DELETE(x) if(x != NULL){ delete x; x = NULL;}

template<typename TYP>
inline double linearInterpolate(TYP v0, TYP v1, double t)
{
	return (TYP)( (double)(1-t)*v0 + (double)t*v1 );
}


enum FractalState{
	FS_CONSTRUCTED = 0,
	FS_PARAMS_PROCESSED,
	FS_RENDERING,
	FS_DONE,

	FS_ERR,
	FS_TIMEOUT,
	FS_CANCEL
};

class Fractal{

public:
	Fractal(unsigned int id, ParamsFile *params, ParamsFileNotSchema *paramsOut);
	virtual ~Fractal();

	bool isEndedEarly();

	bool isOkay();
	std::string getErrors();

	virtual void processParams();
	virtual void render(int maxTime);

	unsigned int getId();

	void doCancel();
	void err(std::string error);

	bool updateStatus(std::string stage, double percent);

	FractalState getState();

protected:
	ParamsFile *p;
	ParamsFileNotSchema *pOut;
	std::fstream flogFile;

	unsigned long renderStart;
	void postRender();

	unsigned long processParamStart;
	void postProcessParams();

private:
	std::stringstream errs;
	unsigned int fId;
	FractalState state;
	unsigned long timeMustStop; // when time() >= this (and timeMustStop != 0), we need to abort
	std::string stageName;
};



#endif /* FRACTAL_H_ */
