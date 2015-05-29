/*
 * FractalLogger.h
 *
 *  Created on: May 28, 2015
 *      Author: jeffrey
 */

#ifndef SERVER_SRC_FRACTALLOGGER_H_
#define SERVER_SRC_FRACTALLOGGER_H_

#include <map>
#include <fstream>
#include <string>
#include <algorithm>
#include "DBManager.h"

#define MAX_FILE_CACHE 12

struct LogCache{

	LogCache(){
		open = false;
		fid = 0;
	}

	std::fstream file;
	int fid;
	bool open;
};

class FractalLogger {
public:

	static FractalLogger *getSingleton();

	void write(int fid, std::string data);

private:

	FractalLogger();
	virtual ~FractalLogger();

	LogCache F[MAX_FILE_CACHE];

	static FractalLogger singleton;

};

#endif /* SERVER_SRC_FRACTALLOGGER_H_ */
