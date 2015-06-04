/*
 * DirectoryManager.h
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#ifndef DIRECTORYMANAGER_H_
#define DIRECTORYMANAGER_H_

#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include "json/json.h"

class DirectoryManager {
public:

	void initialize(std::string arg0);


	std::string getRootDirectory();
	Json::Value &getConfigRoot();

	std::string getFFmpegBinary();

	static DirectoryManager *getSingleton();

private:
	DirectoryManager();
	~DirectoryManager();

	std::string rootDir;
	Json::Value configRoot;

	static DirectoryManager singleton;
};

#endif /* DIRECTORYMANAGER_H_ */
