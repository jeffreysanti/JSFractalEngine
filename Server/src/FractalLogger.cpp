/*
 * FractalLogger.cpp
 *
 *  Created on: May 28, 2015
 *      Author: jeffrey
 */

#include "FractalLogger.h"
#include "FractalGen.h"


FractalLogger FractalLogger::singleton;

FractalLogger::FractalLogger() {
}

FractalLogger::~FractalLogger() {
	for(int i=0; i<MAX_FILE_CACHE; i++){
		if(F[i].open){
			F[i].file.close();
		}
	}
}

FractalLogger *FractalLogger::getSingleton(){
	return &singleton;
}

void FractalLogger::write(int fid, std::string data)
{
	int bucket = fid % MAX_FILE_CACHE;

	if(F[bucket].open && F[bucket].fid == fid){
		F[bucket].file << data;
		F[bucket].file.flush();
	}
	else if(F[bucket].open){
		F[bucket].file.close();
		F[bucket].fid = fid;
		F[bucket].file.open(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".log").c_str(), std::ios::app);
		F[bucket].file << data;
		F[bucket].file.flush();
	}else{
		F[bucket].file.open(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".log").c_str(), std::ios::app);
		F[bucket].fid = fid;
		F[bucket].open = true;
		F[bucket].file << data;
		F[bucket].file.flush();
	}
}

