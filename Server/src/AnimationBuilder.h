/*
 * AnimationBuilder.h
 *
 *  Created on: May 27, 2015
 *      Author: jeffrey
 */

#ifndef SERVER_SRC_ANIMATIONBUILDER_H_
#define SERVER_SRC_ANIMATIONBUILDER_H_

#include "Fractal.h"
#include "DBManager.h"
#include "FractalGenTrackManager.h"
#include "FractalLogger.h"

struct Animation{
public:

	unsigned int baseID;
	unsigned int frames;
	bool manualQueue;
	int maxThreads;

	unsigned long timeMustStop;

	std::list<std::string> frameQueue;
	std::set<int> framesRendering;
};


class AnimationBuilder {
public:
	AnimationBuilder(ParamsFile *params, ParamsFileNotSchema *paramsOut, int fid);
	virtual ~AnimationBuilder();

	Animation spawnJobs(std::string &err, int maxTime);

protected:
	ParamsFile *p;
	ParamsFileNotSchema *pOut;
	int id;
};

#endif /* SERVER_SRC_ANIMATIONBUILDER_H_ */
