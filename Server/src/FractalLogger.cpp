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

	for(int i=0; i<MAX_FILE_CACHE; i++){
		if(P[i].open && P[i].dirty){
			Json::StyledStreamWriter writer;
			std::fstream f(std::string(concat(FractalGen::getSaveDir()+"/",P[i].fid) +".info").c_str(),
					std::fstream::out | std::fstream::trunc);
			writer.write(f, P[i].root);
			f.close();
		}
	}
}

void FractalLogger::forceFlushAll(){
	for(int i=0; i<MAX_FILE_CACHE; i++){
		if(P[i].open && P[i].dirty){
			P[i].lock.lock();
			Json::StyledStreamWriter writer;
			std::fstream f(std::string(concat(FractalGen::getSaveDir()+"/",P[i].fid) +".info").c_str(),
					std::fstream::out | std::fstream::trunc);
			writer.write(f, P[i].root);
			f.close();
			P[i].lock.unlock();
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

Json::Value &FractalLogger::outParams(int fid){
	int bucket = fid % MAX_FILE_CACHE;
	if(P[bucket].open && P[bucket].fid == fid){
		P[bucket].lock.lock();

		P[bucket].dirty = true;
		return P[bucket].root;
	}
	else if(P[bucket].open){
		P[bucket].lock.lock();

		// write old
		Json::StyledStreamWriter writer;
		std::fstream f(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".info").c_str(),
				std::fstream::out | std::fstream::trunc);
		writer.write(f, P[bucket].root);
		f.close();

		// read new
		P[bucket].fid = fid;
		Json::Reader reader;
		std::ifstream f2(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".info").c_str());
		if(f2.good()) {
			if (!reader.parse(f2, P[bucket].root)){
				P[bucket].root = Json::Value(Json::objectValue);
			}
		} else {
			P[bucket].root = Json::Value(Json::objectValue);
		}
		f2.close();
		P[bucket].dirty = true;
		return P[bucket].root;
	}else{
		P[bucket].lock.lock();

		// read new
		P[bucket].fid = fid;
		P[bucket].open = true;
		Json::Reader reader;
		std::ifstream f2(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".info").c_str());
		if(f2.good()) {
			if (!reader.parse(f2, P[bucket].root)){
				P[bucket].root = Json::Value(Json::objectValue);
			}
		} else {
			P[bucket].root = Json::Value(Json::objectValue);
		}
		f2.close();
		P[bucket].dirty = true;
		return P[bucket].root;
	}
}

void FractalLogger::unlockOutParams(int fid, bool flushNow){
	int bucket = fid % MAX_FILE_CACHE;
	if(P[bucket].open && P[bucket].fid == fid){
		if(flushNow){
			P[bucket].dirty = false;
			Json::StyledStreamWriter writer;
			std::fstream f(std::string(concat(FractalGen::getSaveDir()+"/",fid) +".info").c_str(),
					std::fstream::out | std::fstream::trunc);
			writer.write(f, P[bucket].root);
			f.close();
		}
		P[bucket].lock.unlock();
	}
}





