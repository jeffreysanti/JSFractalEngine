/*
 * DirectoryManager.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#include "DirectoryManager.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

DirectoryManager DirectoryManager::singleton;

DirectoryManager::DirectoryManager() {
	rootDir = "";
}

DirectoryManager::~DirectoryManager() {
}

std::string DirectoryManager::getRootDirectory()
{
	if(rootDir == ""){
		std::cerr << "Root Directory Not Set\n";
		exit(EXIT_FAILURE);
		return "";
	}

	return rootDir;
}

inline std::string GetExecutableLocation()
{
   char buf[1024];
   memset(&buf, 0, 1024);

#ifdef _WIN32
   GetModuleFileNameA(NULL, buf, sizeof(buf));
#else
   readlink("/proc/self/exe", buf, sizeof(buf));
#endif
   return std::string(buf);
}

void DirectoryManager::initialize(std::string arg0)
{
	if(rootDir != ""){
		std::cerr << "DirectoryManager::initialize called multiple times!\n";
		exit(EXIT_FAILURE);
		return;
	}

	rootDir = std::string(GetExecutableLocation());
	unsigned dirMarker = rootDir.find_last_of("/\\");
	rootDir = rootDir.substr(0,dirMarker) + "/";


	// find config file
	std::string configFile = rootDir + "servercfg.json";

	Json::Reader reader;
	std::ifstream stream(configFile, std::ifstream::in);
	if (!reader.parse(stream, configRoot))
	{
		stream.close();
		std::cerr << "Error in configuration initialization [DirectoryManager::initialize]!\n";
		std::cerr << "   File failed to parse: '" << configFile << "'\n";
		std::cerr << "   Does this file not exist, or is json invalid?\n";
		std::cerr << reader.getFormattedErrorMessages();
		exit(EXIT_FAILURE);
		return;
	}
	stream.close();

	// find if another directory is set to be the fractal data path
	std::string newDataPath = configRoot.get("directory.data", "").asString();
	if(newDataPath != ""){
		rootDir = newDataPath;
	}
}

Json::Value &DirectoryManager::getConfigRoot()
{
	return configRoot;
}



DirectoryManager *DirectoryManager::getSingleton(){
	return &singleton;
}
