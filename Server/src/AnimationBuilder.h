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
	unsigned int fps;
	bool manualQueue;
	int maxThreads;

	unsigned long timeMustStop;
	unsigned long timeStarted;

	unsigned long timeMuxStarted;

	std::list<std::string> frameQueue;
	std::set<int> framesRendering;
};

struct KeyFrame{
	int frameno;
	Json::Value val;
	std::string interp;

    bool operator < (const KeyFrame& comp) const
    {
        return (frameno < comp.frameno);
    }
};

struct AnimatedParam{
	std::string param;
	SchemaValType pType;
	Json::Value *jsonPtr;

	std::vector<KeyFrame> F;

	int lastSetIndex;
};

class FractalGen;

class AnimationBuilder {
public:
	AnimationBuilder(ParamsFile *params, int fid);
	virtual ~AnimationBuilder();

	Animation spawnJobs(std::string &err, int maxTime);

	static void muxFrames(Animation A, FractalGen *gen);

protected:

	void buildAnimatedParams(Json::Value &anim, ParamsFile *pnew);
	void interpolateFrame(ParamsFile &pnew, int frameno);

	ParamsFile *p;
	int id;

	std::map<std::string, AnimatedParam> A;
};

#endif /* SERVER_SRC_ANIMATIONBUILDER_H_ */
