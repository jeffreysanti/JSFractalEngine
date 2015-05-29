/*
 * AnimationBuilder.cpp
 *
 *  Created on: May 27, 2015
 *      Author: jeffrey
 */

#include "AnimationBuilder.h"
#include "FractalGen.h"

AnimationBuilder::AnimationBuilder(ParamsFile *params, ParamsFileNotSchema *paramsOut, int fid) {
	p = params;
	pOut = paramsOut;
	id = fid;


}

AnimationBuilder::~AnimationBuilder() {
}

Animation AnimationBuilder::spawnJobs(std::string &err, int maxTime){
	FractalLogger::getSingleton()->write(id, "Fractal Is Animated: Note Detailed progress not reported.\n");

	Animation anim;
	anim.baseID = id;
	if(maxTime > 0){
		anim.timeMustStop = time(NULL) + maxTime;
	}else{
		anim.timeMustStop = 0;
	}

	if(!p->getJson().isMember("anim") || !p->getJson()["anim"].isObject()){
		err += "No JSON Object Anim\n";
		return anim;
	}
	Json::Value animData = p->getJson()["anim"];
	p->getJson()["anim"] = Json::ValueType::nullValue;
	p->getJson()["basic"]["anim"] = "no";
	if(!animData.isMember("frames") || !animData["frames"].isInt()){
		err += "anim.frames does not exist or non-int\n";
		return anim;
	}
	anim.frames = animData["frames"].asInt();

	// first job will render frame one -- we need to render all others
	for(int i=2; i<=anim.frames; i++){
		p->getJson()["internal"]["thisframe"] = i;
		std::string savepath = DirectoryManager::getSingleton()->getRootDirectory()+"renders/";
		savepath = concat(savepath, anim.baseID) + concat(".frame.", i) + ".job";

		p->saveToFile(savepath);
		anim.frameQueue.push_back(savepath);
	}

	p->getJson()["internal"]["thisframe"] = 1;
	p->getJson()["anim"] = animData;

	return anim;
}

