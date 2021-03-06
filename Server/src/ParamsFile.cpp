/*
 * ParamsFile.cpp
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#include "ParamsFile.h"

ParamsFile::ParamsFile(const std::string &fileName, bool isFile) {
	open_correct = true;
	open_err = "";

	Json::Reader reader;
	if(isFile){
		std::ifstream stream(fileName, std::ifstream::in);
		if (!reader.parse(stream, root))
		{
			open_correct = false;
			open_err = 	std::string("Error in paramater initialization!\n") +
						std::string("   File failed to parse: '") + fileName + std::string("'\n") +
						std::string("   Does this file not exist, or is json invalid?\n") +
						reader.getFormattedErrorMessages() + std::string("\n");
			root = Json::Value(Json::objectValue);
		}
		if(stream.is_open())
			stream.close();
	}else{
		if(!reader.parse(fileName, root)){
			open_correct = false;
			open_err = 	std::string("Error in paramater initialization!\n") +
						std::string("   String failed to parse:\n") +
						std::string("   json invalid:\n") +
						reader.getFormatedErrorMessages() + std::string("\n");
			root = Json::Value(Json::objectValue);
		}
	}
	if(!root.isObject()){
		open_correct = false;
		open_err = 	std::string("Error in paramater initialization!\n") +
					std::string("   root is not object type\n");
		root = Json::Value(Json::objectValue);
	}
}

ParamsFile::~ParamsFile() {
	// TODO Auto-generated destructor stub
}

std::string ParamsFile::getFormattedJson(){
	Json::StyledWriter writer;
	return writer.write(root);
}

void ParamsFile::saveToFile(std::string path)
{
	Json::StyledStreamWriter writer;
	std::ofstream stream(path, std::ofstream::out);
	if(!stream.is_open()){
		stream.close();
		std::cerr << "Error in paramater saving!\n";
		std::cerr << "   File failed to write: '" << path << "'\n";
		exit(EXIT_FAILURE);
		return;
	}
	writer.write(stream, root);
	stream.close();
}

bool ParamsFile::openedCorrectly()
{
	return open_correct;
}

bool ParamsFile::validate(std::string &err)
{
	if(!open_correct){
		err = open_err;
		return false;
	}
	err = SchemaManager::getSingleton()->validateParamaters(root);
	if(err == ""){
		return true;
	}
	return false;
}

Json::Value &ParamsFile::getJson(){
	return root;
}

std::string ParamsFile::readAllFile(std::string path)
{
	std::ifstream stream(path, std::ifstream::in);
	if (!stream.is_open())
	{
		return "";
	}else{
		std::stringstream buffer;
		buffer << stream.rdbuf();
		return buffer.str();
	}
}

void ParamsFile::clearFrameData(){
	if(!root.isMember("internal") || !root["internal"].isObject())
		return;
	if(root["internal"].isMember("thisframe"))
		root["internal"].removeMember("thisframe");
}

int ParamsFile::getFrameData(){
	if(!root.isMember("internal") || !root["internal"].isObject())
		return 0;
	if(root["internal"].isMember("thisframe") && root["internal"]["thisframe"].isInt())
		return root["internal"]["thisframe"].asInt();
	return 0;
}

int ParamsFile::getID(){
	if(!root.isMember("internal") || !root["internal"].isObject())
		return 0;
	if(root["internal"].isMember("id") && root["internal"]["id"].isInt())
		return root["internal"]["id"].asInt();
	return 0;
}





ParamsFileNotSchema::ParamsFileNotSchema(const std::string &fileName, bool isFile) :
	ParamsFile(fileName, isFile)
{

}

ParamsFileNotSchema::ParamsFileNotSchema() : ParamsFile("", false)
{
	open_correct = true;
	open_err = "";

	root = Json::Value(Json::objectValue);
}

ParamsFileNotSchema::~ParamsFileNotSchema() {
	// TODO Auto-generated destructor stub
}

bool ParamsFileNotSchema::validate(std::string &err)
{
	if(!open_correct){
		err = open_err;
		return false;
	}
	err = "";
	return true;
}



