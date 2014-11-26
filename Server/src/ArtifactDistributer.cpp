/*
 * ArtifactDistributer.cpp
 *
 *  Created on: Nov 26, 2014
 *      Author: jeffrey
 */

#include "ArtifactDistributer.h"
#include "ImageWriter.h"
#include "DBManager.h"

#include "FractalMandleJulia.h"

ArtifactDistributer ArtifactDistributer::singleton;


ArtifactDistributer::ArtifactDistributer() {
	// TODO Auto-generated constructor stub

}

ArtifactDistributer::~ArtifactDistributer() {
	// TODO Auto-generated destructor stub
}

ArtifactDistributer *ArtifactDistributer::getSingleton(){
	return &singleton;
}


// Artifact Data [#artifacts:4]
// Each Artiface: [XXXX:4][len:4][DTA]
unsigned int ArtifactDistributer::getArtifactData(FractalMeta &meta, char **dta)
{
	std::string type = getFractType(meta);
	if(type == "mandlejulia"){
		return FractalMandleJulia::returnArtifacts(meta, dta);
	}else{
		// no artifacts, therefore only count of four bytes

		*dta = new char[4];
		char *ptr = *dta;

		memset(ptr, 0, 4);
		return 4;
	}
}


FILE * ArtifactDistributer::getFractalThumbnail(FractalMeta &meta)
{
	std::string type = getFractType(meta);
	if(type == "mandlejulia"){
		std::string imgPath = concat(DirectoryManager::getSingleton()->getRootDirectory()+"renders/", meta.jobID) + ".png";
		ImageWriter img(100, 100);
		return img.createThumbnail(imgPath);
	}else{
		return nullptr;
	}
}

std::string ArtifactDistributer::getFractType(FractalMeta &meta)
{
	// load paramaters
	ParamsFile params(concat(DirectoryManager::getSingleton()->getRootDirectory()+"renders/", meta.jobID) + ".job", true);
	std::string err = "";
	if(!params.openedCorrectly()){
		return "";
	}
	if(!params.getJson().isMember("basic") || !params.getJson()["basic"].isObject() ||
			!params.getJson()["basic"].isMember("type") || !params.getJson()["basic"]["type"].isObject() ||
			!params.getJson()["basic"]["type"].isMember("selected") ||
			!params.getJson()["basic"]["type"]["selected"].isString()){
		return "";
	}
	std::string type = params.getJson()["basic"]["type"]["selected"].asString();
	return type;
}


