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
	if(!root.isObject() || !root.isMember("groups") || !root["groups"].isArray() || root["groups"].empty()){
		std::cerr << "Schema must contain array of groups\n";
		exit(EXIT_FAILURE);
		return;
	}

	for(auto grp : root["groups"]){
		std::string gname = grp["id"].asString();

		if(!grp.isObject() || !grp.isMember("elms") || grp["elms"].size() < 1)
			continue;

		G[gname] = grp;
	}
}
/*
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
*/
void SchemaManager::findConditionlessGroups(std::deque<std::string> &lst)
{
	for(auto it=G.begin(); it!=G.end(); ++it){
		if((*it).second.isMember("active") && (*it).second.asBool()){
			lst.push_back((*it).first);
		}
	}
}

std::string SchemaManager::validateParamaters(Json::Value paramRoot)
{
	// different element types
	SchemaElementIntegral Eint;
	SchemaElementText Etxt;

	std::string ret = ""; // no errors
	std::deque<std::string> queue;
	std::set<std::string> parsed; // store already evaluated groups
	findConditionlessGroups(queue); // get initial search list

	for(std::string ent : queue){
		parsed.insert(ent);
	}

	while(queue.size() > 0){
		std::string id = queue.front();
		queue.pop_front();

		// validate this group
		if(!paramRoot.isObject() || !paramRoot.isMember(id) || !paramRoot[id].isObject()){
			ret = "Necessary Group: " + id + " does not exist!\n";
			return ret;
		}
		if(G.find(id) == G.end()){
			ret = "Group: " + id + " does not exist in schema!\n";
			return ret;
		}
		Json::Value group = G[id]["elms"];
		for(auto elm : group){
			// validate this element
			std::set<std::string> actuators;

			if(elm["type"].asString() == "integer"){
				Eint.verifyElement(elm, paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "text"){
				Etxt.verifyElement(elm, paramRoot[id], ret, actuators);
			}else{
				return "Unknown element type: " + elm["type"].asString();
			}

			if(ret != ""){
				return ret;
			}
			for(std::string readGroup: actuators){
				if(parsed.find(readGroup) != parsed.end()){
					queue.push_back(readGroup);
					parsed.insert(readGroup);
				}
			}
		}
	}

	return ret;
}

SchemaManager *SchemaManager::getSingleton()
{
	return &singleton;
}
