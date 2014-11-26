/*
 * ParamsFile.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef PARAMSFILE_H_
#define PARAMSFILE_H_

#include "SchemaManager.h"
#include <sstream>

class ParamsFile {
public:
	ParamsFile(const std::string &fileName, bool isFile=false);
	virtual ~ParamsFile();

	Json::Value &getJson();
	virtual bool validate(std::string &err);
	bool openedCorrectly();
	void saveToFile(std::string path);

	std::string getFormattedJson();

	static std::string readAllFile(std::string path);

protected:
	Json::Value root;
	bool open_correct;
	std::string open_err;
};

class ParamsFileNotSchema : public ParamsFile {
public:
	ParamsFileNotSchema(const std::string &fileName, bool isFile=false);
	ParamsFileNotSchema();
	virtual ~ParamsFileNotSchema();

	virtual bool validate(std::string &err);
};

#endif /* PARAMSFILE_H_ */
