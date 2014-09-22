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

	// now load the schema
	if(!root.isMember("groups") || !root["groups"].isArray() || root["groups"].empty()){
		std::cerr << "Schema must contain array of groups\n";
		exit(EXIT_FAILURE);
		return;
	}
	for(auto grp : root["groups"]){
		std::string gname = grp["id"].asString();
		auto it = G.find(gname);
		if(it != G.end()){
			for(auto elm : grp["elms"]){
				(*it).second.E.push_back(parseElement(elm));
			}
		}else{
			Group g;
			for(auto elm : grp["elms"]){
				g.E.push_back(parseElement(elm));
			}
			G[gname] = g;
		}
	}
}

Element SchemaManager::parseElement(Json::Value elm)
{
	std::string id = elm["id"].asString();
	std::string type = elm["type"].asString();
	bool allowNull = elm["allowNull"].asBool();

	NumericConstraintIntegral NC_i;
	if(elm.isMember("min")){
		NC_i.minimized = true;
		NC_i.min = elm["min"].asLargestInt();
	}else
		NC_i.minimized = false;
	if(elm.isMember("max")){
		NC_i.maximized = true;
		NC_i.max = elm["max"].asLargestInt();
	}else
		NC_i.maximized = false;

	NumericConstraintIntegral NC_r;
	if(elm.isMember("min")){
		NC_r.minimized = true;
		NC_r.min = elm["min"].asDouble();
	}else
		NC_r.minimized = false;
	if(elm.isMember("max")){
		NC_r.maximized = true;
		NC_r.max = elm["max"].asDouble();
	}else
		NC_r.maximized = false;


	if(type == "text"){
		ElementText e;
		e.name = id;
		e.allowNull = allowNull;
		e.NC = NC_i;

		return e;
	}else if(type == "integer"){
		ElementInteger e;
		e.name = id;
		e.allowNull = allowNull;
		e.NC = NC_i;

		return e;
	}

	Element e;
	e.name = id;
	e.allowNull = allowNull;
	return e;
}

std::string SchemaManager::validateParamaters(Json::Value paramRoot)
{
	std::string ret = ""; // no errors

	return ret;
}

SchemaManager *SchemaManager::getSingleton()
{
	return &singleton;
}
