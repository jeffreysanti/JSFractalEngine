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
	FractalLogger::getSingleton()->write(id, "Building Animation Data...\n");


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

	if(!animData.isMember("keyframes") || !animData["keyframes"].isArray()){
		err += "anim.keyframes does not exist or non-array\n";
		return anim;
	}
	if(!SchemaManager::getSingleton()->validateAnimationParam(animData["keyframes"], anim.frames, err)){
		err += "Keyframe Validation Reported Error(s)!\n";
		return anim;
	}

	// first job will render frame one -- we need to render all others
	ParamsFile pnew(p->getJson().toStyledString(), false); // copy json data to interpolate
	buildAnimatedParams(animData, &pnew);

	for(int i=2; i<=anim.frames; i++){
		pnew.getJson()["internal"]["thisframe"] = i;
		std::string savepath = DirectoryManager::getSingleton()->getRootDirectory()+"renders/";
		savepath = concat(savepath, anim.baseID) + concat(".frame.", i) + ".job";

		// zoom
		//pnew.getJson()["type.juliamandle"]["radI"] = pnew.getJson()["type.juliamandle"]["radI"].asDouble() * 0.98;
		//pnew.getJson()["type.juliamandle"]["radR"] = pnew.getJson()["type.juliamandle"]["radR"].asDouble() * 0.98;

		interpolateFrame(pnew, i);


		pnew.saveToFile(savepath);
		anim.frameQueue.push_back(savepath);
	}

	p->getJson()["internal"]["thisframe"] = 1;
	p->getJson()["anim"] = animData;

	// finally revalidate the parameters in case we messed up
	err += SchemaManager::getSingleton()->validateParamaters(p->getJson());

	if(err == "")
		FractalLogger::getSingleton()->write(id, "Animation Data Built!\n");
	return anim;
}


void AnimationBuilder::buildAnimatedParams(Json::Value &anim, ParamsFile *pnew){
	A.clear();

	for(auto keyframe : anim["keyframes"]){
		int fno = keyframe["frame"].asInt();
		Json::Value val = keyframe["val"];
		std::string param = keyframe["param"].asString();
		std::string interp = keyframe["interp"].asString();

		KeyFrame kf;
		kf.frameno = fno;
		kf.val = val;
		kf.interp = interp;

		if(A.find(param) != A.end()){
			A[param].F.push_back(kf);
		}else{
			AnimatedParam ap;
			ap.param = param;
			ap.lastSetIndex = 0;
			SchemaValTypeJsonObj typePtr = SchemaManager::getSingleton()->getParamAddrType(param, &pnew->getJson());
			ap.jsonPtr = typePtr.param;
			ap.pType = typePtr.vt;

			if(ap.jsonPtr == NULL){
				FractalLogger::getSingleton()->write(id, "Warning: Parameter "+param+" cannot be" +
						" animated because it was not found in job file.\n");
				continue; // this value cannot be changed
			}
			KeyFrame kfi;
			kfi.frameno = 1;
			kfi.val = *ap.jsonPtr;
			kfi.interp = "none";

			ap.F.push_back(kfi);
			ap.F.push_back(kf);

			A[param] = ap;
		}
	}

	// sort so that all keyframes in order
	for(auto ap : A){
		std::sort(ap.second.F.begin(), ap.second.F.end());
	}
}

template<typename T>
T animationInterpolate(std::string method, int frame, int fi, int ff, T i, T f){
	double steps = ff - fi;
	double progress = double(frame-fi) / steps;
	if(method == "linear"){
		T m = f - i;
		T b = i;
		return (double)m * progress + b;
	}
	if(method == "square"){
		T m = f - i;
		T b = i;
		return (double)m * std::pow(progress,2) + b;
	}
	if(method == "cube"){
		T m = f - i;
		T b = i;
		return (double)m * std::pow(progress,3) + b;
	}
	if(method == "sqroot"){
		T m = f - i;
		T b = i;
		return (double)m * std::sqrt(progress) + b;
	}
	if(method == "cuberoot"){
		T m = f - i;
		T b = i;
		return (double)m * std::pow(progress,(double)1/3) + b;
	}

	return i;
}

void AnimationBuilder::interpolateFrame(ParamsFile &pnew, int frameno)
{
	for(auto itP = A.begin(); itP != A.end(); itP ++){
		if((*itP).second.lastSetIndex >= (*itP).second.F.size() - 1){
			continue;
		}

		int nextFrameIndex = (*itP).second.lastSetIndex + 1;
		KeyFrame nextFrame = (*itP).second.F[nextFrameIndex];

		// TODO: Interpolate if not on frame
		if(nextFrame.frameno == frameno){
			*((*itP).second.jsonPtr) = nextFrame.val;

			(*itP).second.lastSetIndex = nextFrameIndex;
		}else if(nextFrame.interp != "none"){
			int fi = (*itP).second.F[(*itP).second.lastSetIndex].frameno;
			int ff = nextFrame.frameno;
			Json::Value i = (*itP).second.F[(*itP).second.lastSetIndex].val;
			Json::Value f = nextFrame.val;

			if((*itP).second.pType == SVT_INT){
				*((*itP).second.jsonPtr) =
					animationInterpolate(nextFrame.interp, frameno, fi, ff, i.asInt(), f.asInt());
			}else if((*itP).second.pType == SVT_REAL){
				*((*itP).second.jsonPtr) =
					animationInterpolate(nextFrame.interp, frameno, fi, ff, i.asDouble(), f.asDouble());
			}else if((*itP).second.pType == SVT_COLOR){
				Json::Value arr = Json::Value(Json::ValueType::arrayValue);
				arr.resize(3);
				arr[0] = animationInterpolate(nextFrame.interp, frameno, fi, ff,
							i[0].asDouble(), f[0].asDouble());
				arr[1] = animationInterpolate(nextFrame.interp, frameno, fi, ff,
							i[1].asInt(), f[1].asInt());
				arr[2] = animationInterpolate(nextFrame.interp, frameno, fi, ff,
							i[2].asInt(), f[2].asInt());
				*((*itP).second.jsonPtr) = arr;
			}
		}
	}
}

