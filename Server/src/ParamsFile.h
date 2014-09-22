/*
 * ParamsFile.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef PARAMSFILE_H_
#define PARAMSFILE_H_

#include "SchemaManager.h"

class ParamsFile {
public:
	ParamsFile(std::string fileName);
	virtual ~ParamsFile();

private:
	Json::Value root;
};

#endif /* PARAMSFILE_H_ */
