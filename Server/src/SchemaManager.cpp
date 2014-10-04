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

		if(!grp.isObject() || !grp.isMember("elms") || !grp["elms"].isArray())
			continue;

		G[gname] = grp;
	}
}

void SchemaManager::findConditionlessGroups(std::deque<std::string> &lst)
{
	for(auto it=G.begin(); it!=G.end(); ++it){
		if((*it).second.isMember("active") && (*it).second.asBool()){
			lst.push_back((*it).first);
		}
	}
}

inline void insertMatchesFromGroupList(const std::map<std::string, Json::Value> &G,
		std::deque<std::string> &queue, std::set<std::string> &set, std::vector<SchemaActuator> &A){
	std::set<std::string> R;
	for(SchemaActuator act : A){
		if(act.action == "hide"){
			if(act.match.at(act.match.length()-1) != '*'){
				R.insert(act.match);
				continue;
			}
			std::string tmp = act.match.substr(0, act.match.length()-1);
			for(auto it : G){
				if(it.first.find(tmp) != std::string::npos){
					R.insert(it.first);
				}
			}
		}
	}
	// now find the "adds"
	for(SchemaActuator act : A){
		if(act.action == "show"){
			if(act.match.at(act.match.length()-1) != '*'){
				queue.push_back(act.match);
				R.erase(act.match);
				continue;
			}
			std::string tmp = act.match.substr(0, act.match.length()-1);
			for(auto it : G){
				if(it.first.find(tmp) != std::string::npos){
					queue.push_back(it.first);
					R.erase(it.first);
				}
			}
		}
	}

	// now truly remove remaining in R
	for(std::string r : R){
		set.insert(r);
	}
}

std::string SchemaManager::validateParamaters(Json::Value &paramRoot)
{
	std::string ret = ""; // no errors
	std::deque<std::string> queue;
	std::set<std::string> parsed; // store already evaluated groups
	findConditionlessGroups(queue); // get initial search list

	while(queue.size() > 0){
		std::string id = queue.front();
		queue.pop_front();

		if(parsed.find(id) != parsed.end()) // already seen this
			continue;
		parsed.insert(id);

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
			std::vector<SchemaActuator> actuators;

			if(elm["type"].asString() == "integer"){
				SchemaElementIntegral Eint(id, elm);
				Eint.verifyElement(paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "real"){
				SchemaElementReal Erel(id, elm);
				Erel.verifyElement(paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "text"){
				SchemaElementText Etxt(id, elm);
				Etxt.verifyElement(paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "selector"){
				SchemaElementSelector Esel(id, elm);
				Esel.verifyElement(paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "color"){
				SchemaElementColor Ecol(id, elm);
				Ecol.verifyElement(paramRoot[id], ret, actuators);
			}else if(elm["type"].asString() == "tuple"){
				SchemaElementTuple Etup(id, elm);
				Etup.verifyElement(paramRoot[id], ret, actuators);
			}else{
				return "Unknown element type: " + elm["type"].asString();
			}

			if(ret != ""){
				return ret;
			}
			insertMatchesFromGroupList(G, queue, parsed, actuators);
		}
	}

	return ret;
}

SchemaManager *SchemaManager::getSingleton()
{
	return &singleton;
}
