/*
 * SchemaManager.cpp
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#include "SchemaManager.h"

SchemaManager SchemaManager::singleton;

SchemaManager::SchemaManager() {
	// TODO Auto-generated constructor stub

}

SchemaManager::~SchemaManager() {
	// TODO Auto-generated destructor stub
}


void SchemaManager::initialize()
{
	std::string configFile = DirectoryManager::getSingleton()->getRootDirectory() +
			"schema.json";

	Json::Reader reader;
	std::ifstream stream(configFile, std::ifstream::in);
	if (!reader.parse(stream, root))
	{
		stream.close();
		std::cerr << "Error in schema initialization [SchemaManager::initialize]!\n";
		std::cerr << "   File failed to parse: '" << configFile << "'\n";
		std::cerr << "   Does this file not exist, or is json invalid?\n";
		std::cerr << reader.getFormattedErrorMessages();
		exit(EXIT_FAILURE);
		return;
	}
	stream.close();
}

SchemaManager *SchemaManager::getSingleton()
{
	return &singleton;
}
