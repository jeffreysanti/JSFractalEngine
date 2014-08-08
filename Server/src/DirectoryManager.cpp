/*
 * DirectoryManager.cpp
 *
 *  Created on: Aug 3, 2014
 *      Author: jeffrey
 */

#include "DirectoryManager.h"

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

void DirectoryManager::initialize(std::string arg0)
{
	if(rootDir != ""){
		std::cerr << "DirectoryManager::initialize called multiple times!\n";
		exit(EXIT_FAILURE);
		return;
	}

	// extract path to find config file
	unsigned dirMarker = arg0.find_last_of("/\\");
	rootDir = arg0.substr(0,dirMarker) + "/";
	std::string configFile = rootDir + "servercfg.json";

	Json::Reader reader;
	std::ifstream stream(configFile, std::ifstream::in);
	if (!reader.parse(stream, configRoot))
	{
		stream.close();
		std::cerr << "Error in configuration initialization [DirectoryManager::initialize]!\n";
		std::cerr << "   File failed to parse: " << configFile << "\n";
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
