/*
 * SchemaManager.cpp
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#include "SchemaManager.h"
#include "DBManager.h"

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
	if (!reader.parse(stream, inital_root))
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
	if(!inital_root.isObject() || !inital_root.isMember("groups") || !inital_root["groups"].isArray() || inital_root["groups"].empty()){
		std::cerr << "Schema must contain array of groups\n";
		exit(EXIT_FAILURE);
		return;
	}

	for(auto grp : inital_root["groups"]){
		if(!grp.isObject() || !grp.isMember("elms") || !grp["elms"].isArray() || !grp.isMember("id"))
			continue;

		std::string gname = grp["id"].asString();
		inital_G[gname] = grp;
	}
}

void SchemaManager::findConditionlessGroups(std::deque<std::string> &lst)
{
	int i = 0;
	for(auto it=inital_G.begin(); it!=inital_G.end(); ++it){
		if((*it).second.isMember("active") && (*it).second["active"].isBool() && (*it).second["active"].asBool()){
			i ++;
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

	std::map<std::string, Json::Value> G;
	Json::Reader reader;
	for(auto ent : inital_G){
		Json::Value cpy;
		reader.parse(ent.second.toStyledString(), cpy);
		G[ent.first] = cpy;
	}

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
			}else if(elm["type"].asString() == "complex"){
				SchemaElementComplex Ecomp(id, elm);
				Ecomp.verifyElement(paramRoot[id], ret, actuators);
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
			}else if(elm["type"].asString() == "array"){
				SchemaElementArray Earr(id, elm);
				Earr.verifyElement(paramRoot[id], ret, actuators);
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

SchemaValTypeJsonObj SchemaManager::getParamAddrType(std::string addr, Json::Value *paramRoot)
{
	Json::Value elm;
	Json::Value *param;

	std::vector<std::string> parts = split(addr, ':');
	if(parts.size() < 2) return SchemaValTypeJsonObj(SVT_ERR, elm, param);

	std::string groupstr = parts[0];
	if(inital_G.find(groupstr) == inital_G.end()) return SchemaValTypeJsonObj(SVT_ERR, elm, param);
	Json::Value group = inital_G[groupstr]["elms"];

	if(paramRoot == NULL || !paramRoot->isObject()){
		param = NULL;
	}else{
		param = paramRoot;
	}
	if(param != NULL && param->isMember(groupstr) && (*param)[groupstr].isObject()){
		param = &(*paramRoot)[groupstr];
	}else{
		param = NULL;
	}

	std::string elmstr = parts[1];
	bool found = false;
	for(Json::Value e : group){
		if(e["id"] == elmstr){
			elm = e;
			found = true;

			if(param != NULL && param->isMember(elmstr)){
				param = &(*param)[elmstr];
			}else{
				param = NULL;
			}
		}
	}
	if(!found) return SchemaValTypeJsonObj(SVT_ERR, elm, param);

	int index = 1;
	while(true){
		if(elm["type"] == "text")
			return SchemaValTypeJsonObj(SVT_TEXT, elm, param);
		if(elm["type"] == "integer")
			return SchemaValTypeJsonObj(SVT_INT, elm, param);
		if(elm["type"] == "real")
			return SchemaValTypeJsonObj(SVT_REAL, elm, param);
		if(elm["type"] == "complex")
			return SchemaValTypeJsonObj(SVT_COMPLEX, elm, param);
		if(elm["type"] == "color")
			return SchemaValTypeJsonObj(SVT_COLOR, elm, param);
		if(elm["type"] == "selector"){
			if(index+1 < parts.size()){
				index ++;
				std::string s = parts[index];
				bool found = false;
				for(Json::Value choice : elm["choices"]){
					if(choice["id"] == s && choice.isMember("elm")){
						elm = choice["elm"];
						found = true;
						break;
					}
				}
				if(found){
					if(param != NULL && param->isObject() && param->isMember(s)){
						param = &(*param)[s];
					}else{
						param = NULL;
					}
					continue;
				}
				return SchemaValTypeJsonObj(SVT_ERR, elm, param);
			}else{
				// selector cannot change schema state, so no hide/show/elm in any choice
				for(Json::Value choice : elm["choices"]){
					if(choice.isMember("elm") || choice.isMember("hide") || choice.isMember("show")){
						return SchemaValTypeJsonObj(SVT_ERR, elm, param);
					}
				}
				return SchemaValTypeJsonObj(SVT_SELECTOR_NO_AFFECT, elm, param);
			}
		}
		if(elm["type"] == "tuple"){
			if(index+1 >= parts.size())
				SchemaValTypeJsonObj(SVT_ERR, elm, param);
			index ++;
			std::string s = parts[index];
			bool found = false;
			for(Json::Value telm : elm["elms"]){
				if(telm["id"] == s){
					elm = telm;
					found = true;
					break;
				}
			}
			if(found){
				if(param != NULL && param->isObject() && param->isMember(s)){
					param = &(*param)[s];
				}else{
					param = NULL;
				}
				continue;
			}
			return SchemaValTypeJsonObj(SVT_ERR, elm, param);
		}
		if(elm["type"] == "array"){
			if(index+1 >= parts.size())
				return SchemaValTypeJsonObj(SVT_ERR, elm, param);
			index ++;
			elm = elm["elm"];

			std::string aind = parts[index];
			int aindex = atoi(aind.c_str());
			if(param != NULL && param->isArray() && param->isValidIndex(aindex)){
				param = &(*param)[aindex];
			}else{
				param = NULL;
			}
			continue;
		}
	}

	return SchemaValTypeJsonObj(SVT_ERR, elm, param);
}

Json::Value *SchemaManager::getParamByAddr(Json::Value *paramRoot, std::string addr)
{
	SchemaValTypeJsonObj tmp = getParamAddrType(addr, paramRoot);
	return tmp.param;
}

bool SchemaManager::validateAnimationParam(Json::Value &keyframeRoot, int frameCount, std::string &err)
{
	if(!keyframeRoot.isArray()) // quick assertion - should already be checked
		return false;

	for(auto keyframe : keyframeRoot){
		if(!keyframe.isObject()){
			err += "Error: Keyframe found which is not object!\n";
			return false;
		}
		if(!keyframe.isMember("frame") || !keyframe["frame"].isInt()){
			err += "Error: Keyframe must contain integer 'frame' number!\n";
			return false;
		}
		int frameNo = keyframe["frame"].asInt();
		if(frameNo > frameCount || frameNo <= 1){
			err += "Error: Keyframe frame number must be > 1 and  <= framecount!\n";
			return false;
		}
		if(!keyframe.isMember("val")){
			err += "Error: Keyframe must contain a val element!\n";
			return false;
		}
		if(!keyframe.isMember("param") || !keyframe["param"].isString()){
			err += "Error: Keyframe must contain string 'param'!\n";
			return false;
		}

		if(!keyframe.isMember("interp") || !keyframe["interp"].isString()){
			keyframe["interp"] = "none";
		}
		std::string interp = keyframe["interp"].asString();
		if(interp != "none" && interp != "linear" && interp != "cube" && interp != "square" &&
				interp != "sqroot" && interp != "cuberoot"){
			err += "Error: Invalid Keyframe interpolation: "+interp+"!\n";
			return false;
		}

		// now check the param

		SchemaValTypeJsonObj ret = getParamAddrType(keyframe["param"].asString(), NULL);
		if(ret.vt == SVT_ERR){
			err += "Error: Param Address Invalid!\n";
			return false;
		}
		Json::Value schemaEntry = ret.elm;

		// now verify the data:
		std::vector<SchemaActuator> act;
		if(ret.vt == SVT_INT){
			SchemaElementIntegral Eint("", schemaEntry);
			Eint.overrideElementID("val");
			Eint.verifyElement(keyframe, err, act);
		}else if(ret.vt == SVT_REAL){
			SchemaElementReal Ereal("", schemaEntry);
			Ereal.overrideElementID("val");
			Ereal.verifyElement(keyframe, err, act);
		}else if(ret.vt == SVT_COMPLEX){
			SchemaElementComplex Ecomp("", schemaEntry);
			Ecomp.overrideElementID("val");
			Ecomp.verifyElement(keyframe, err, act);
		}else if(ret.vt == SVT_TEXT){
			SchemaElementText Etext("", schemaEntry);
			Etext.overrideElementID("val");
			Etext.verifyElement(keyframe, err, act);
			if(interp != "none"){
				err += "Error: Text Keyframe cannot be interpolated!\n";
				return false;
			}
		}else if(ret.vt == SVT_COLOR){
			SchemaElementColor Ecolor("", schemaEntry);
			Ecolor.overrideElementID("val");
			Ecolor.verifyElement(keyframe, err, act);
		}else if(ret.vt == SVT_SELECTOR_NO_AFFECT){
			SchemaElementSelector Esel("", schemaEntry);
			Esel.overrideElementID("val");
			Esel.verifyElement(keyframe, err, act);
			if(interp != "none"){
				err += "Error: Selector Keyframe cannot be interpolated!\n";
				return false;
			}
		}

	}
	return err == "";
}



SchemaManager *SchemaManager::getSingleton()
{
	return &singleton;
}
