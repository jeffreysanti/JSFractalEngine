/*
 * ParamsFile.cpp
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#include "ParamsFile.h"

ParamsFile::ParamsFile(std::string fileName) {
	std::string configFile = DirectoryManager::getSingleton()->getRootDirectory() +
				fileName;

	Json::Reader reader;
	std::ifstream stream(configFile, std::ifstream::in);
	if (!reader.parse(stream, root))
	{
		stream.close();
		std::cerr << "Error in paramater initialization!\n";
		std::cerr << "   File failed to parse: '" << configFile << "'\n";
		std::cerr << "   Does this file not exist, or is json invalid?\n";
		std::cerr << reader.getFormattedErrorMessages();
		exit(EXIT_FAILURE);
		return;
	}
	stream.close();

	std::cout << SchemaManager::getSingleton()->validateParamaters(root) << "\n";

	Json::StyledWriter writer;
	std::cout << writer.write(root) << "\n";
	exit(0);
	return;
}

ParamsFile::~ParamsFile() {
	// TODO Auto-generated destructor stub
}

