/*
 * SchemaElement.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: jeffrey
 */

#include "SchemaElement.h"

SchemaElement::SchemaElement(std::string grpAddr, Json::Value &schema) : schm(schema)
{
	if(!schm.isMember("id") || !schm["id"].isString()){
		std::cerr << "SchemaElement Missing ID\n";
		exit(EXIT_FAILURE);
		return;
	}
	elmid = schm["id"].asString();
	addr = grpAddr + "." + elmid;
}
SchemaElement::~SchemaElement()
{

}


bool SchemaElement::allowNull()
{
	if(schm.isMember("allowZero") && !schm["allowZero"].asBool()){
		return false;
	}else{
		return true;
	}
}

template<class T> void SchemaElement::numericConstraints(bool &max, bool &min, T &maxv, T &minv, bool &zero)
{
	zero = allowNull();
	min = false;
	max = false;
	if(schm.isMember("min")){
		min = true;
		minv = schm["min"].asDouble();
	}
	if(schm.isMember("max")){
		max = true;
		maxv = schm["max"].asDouble();
	}
}

bool SchemaElement::isPresent(Json::Value &in)
{
	if(!in.isMember(elmid)){
		return false;
	}
	return true;
}

SchemaElementIntegral::SchemaElementIntegral(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	numericConstraints(maximized, minimized, max, min, allowNullZero);
	if(schm.isMember("default") && schm["default"].isNumeric())
		defVal = schm["default"].asLargestInt();
	else
		defVal = (minimized ? min : 1);
}

void SchemaElementIntegral::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	long long val;
	if(!isPresent(in) || !in[elmid].isNumeric()){
		val = defVal;
	}else{
		val = in[elmid].asLargestInt();
	}
	// now verify numeric constraints
	if(!allowNullZero && val == 0)
		err += "Illegally Zero: " + addr + "\n";
	if(maximized && val > max)
		err += "Maximum Constraint Violated on " + addr + "\n";
	if(minimized && val < min)
		err += "Minimum Constraint Violated on " + addr + "\n";
	in[elmid] = val;
}

SchemaElementReal::SchemaElementReal(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	numericConstraints(maximized, minimized, max, min, allowNullZero);
	if(schm.isMember("default") && schm["default"].isNumeric())
		defVal = schm["default"].asDouble();
	else
		defVal = (minimized ? min : 1);
}

void SchemaElementReal::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	long double val;
	if(!isPresent(in) || !in[elmid].isNumeric()){
		val = defVal;
	}else{
		val = in[elmid].asDouble();
	}
	// now verify numeric constraints
	if(!allowNullZero && val == 0)
			err += "Illegally Zero: " + addr + "\n";
	if(maximized && val > max)
		err += "Maximum Constraint Violated on " + addr + "\n";
	if(minimized && val < min)
		err += "Minimum Constraint Violated on " + addr + "\n";
	in[elmid] = (double)val;
}

SchemaElementText::SchemaElementText(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	numericConstraints(maximized, minimized, max, min, allowNullZero);
	if(schm.isMember("default") && schm["default"].isString())
		defVal = schm["default"].asString();
	else
		defVal = "";
}

void SchemaElementText::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	std::string val;
	if(!isPresent(in) || !in[elmid].isString()){
		val = defVal;
	}else{
		val = in[elmid].asString();
	}
	// now verify numeric constraints
	if(!allowNullZero && val.length() == 0)
			err += "Illegally Zero Length: " + addr + "\n";
	if(maximized && val.length() > max)
		err += "Maximum Constraint[String Length] Violated on " + addr + "\n";
	if(minimized && val.length() < min)
		err += "Minimum Constraint[String Length] Violated on " + addr + "\n";
	in[elmid] = val;
}

SchemaElementSelector::SchemaElementSelector(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	// read options list
	if(!schm.isMember("choices") || !schm["choices"].isArray()){
		std::cerr << "SchemaElement Selector " << addr << " Missing Choices Array\n";
		exit(EXIT_FAILURE);
		return;
	}
	for(Json::Value o : schm["choices"]){
		if(!o.isObject() || !o.isMember("id") || !o["id"].isString()){
			std::cerr << "SchemaElement Selector " << addr << " has invalid choice! \n";
			exit(EXIT_FAILURE);
			return;
		}
		std::vector<SchemaActuator> actuators;
		if(o.isMember("hide") && o["hide"].isArray()){
			for(auto act : o["hide"]){
				if(act.isString()){
					SchemaActuator a;
					a.action = "hide";
					a.match = act.asString();
					actuators.push_back(a);
				}
			}
		}
		if(o.isMember("show") && o["show"].isArray()){
			for(auto act : o["show"]){
				if(act.isString()){
					SchemaActuator a;
					a.action = "show";
					a.match = act.asString();
					actuators.push_back(a);
				}
			}
		}
		O[o["id"].asString()] = actuators;
	}

	if(schm.isMember("default") && schm["default"].isString())
		defVal = schm["default"].asString();
	else
		defVal = "";
}

void SchemaElementSelector::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	std::string val;
	if(!isPresent(in) || !in[elmid].isString()){
		val = defVal;
	}else{
		val = in[elmid].asString();
	}
	// now verify constraints
	if(O.find(val) == O.end())
		err += "Illegal Selector Option Chosen ["+val+"] for " + addr + "\n";
	else
		actutators = O[val];
	in[elmid] = val;
}

SchemaElementColor::SchemaElementColor(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	if(schm.isMember("default") && schm["default"].isArray() && schm["default"].size() >= 3){
		defR = schm["default"][0].asUInt();
		defG = schm["default"][1].asUInt();
		defB = schm["default"][2].asUInt();
	}else{
		defR = defB = defG = 128;
	}
}

void SchemaElementColor::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	unsigned char val[3];
	if(!isPresent(in) || !in[elmid].isArray() || in[elmid].size() < 3){
		val[0] = defR;
		val[1] = defG;
		val[2] = defB;
	}else{
		if( !in[elmid][0].isNumeric() ||
			!in[elmid][1].isNumeric() ||
			!in[elmid][2].isNumeric()){
			err += "Invalid Color Array on " + addr + "\n";
		}else{
			val[0] = in[elmid][0].asUInt();
			val[1] = in[elmid][1].asUInt();
			val[2] = in[elmid][2].asUInt();
		}
	}
	in[elmid][0] = val[0];
	in[elmid][1] = val[1];
	in[elmid][2] = val[2];
}

/*
SchemaElementColorIterMax::SchemaElementColorIterMax(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
}

void SchemaElementColor::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	if(!isPresent(in) || !in[elmid].isMember("col") || !in[elmid]["col"].isArray() ||
			in[elmid]["col"].size() < 3 ||
			!in[elmid].isMember("iterMax") || !in[elmid]["iterMax"].isNumeric()){
		Json::Value val;
		val["iterMax"] = -1;
		Json::Value colArray;
		colArray[0] = 128;
		colArray[1] = 128;
		colArray[2] = 128;
		val["col"] = colArray;
		in[elmid] = val;
	}else{
		if( !in[elmid]["col"][0].isNumeric() ||
				!in[elmid]["col"][1].isNumeric() ||
				!in[elmid]["col"][2].isNumeric() ||
				!in[elmid]["maxIter"].isNumeric()){
			err += "Invalid Values For " + addr + "\n";
		}
	}
}
*/


SchemaElementTuple::SchemaElementTuple(std::string grpAddr, Json::Value &schema) :
		SchemaElement(grpAddr, schema)
{
	if(schm.isMember("elms") && schm["elms"].isArray()){
		for(auto elm : schm["elms"]){
			if(elm["type"].asString() == "integer"){
				SchemaElement *Eint = new SchemaElementIntegral(addr, elm);
				E.push_back(Eint);
			}else if(elm["type"].asString() == "real"){
				SchemaElement * Erel = new SchemaElementReal(addr, elm);
				E.push_back(Erel);
			}else if(elm["type"].asString() == "text"){
				SchemaElement * Etxt = new SchemaElementText(addr, elm);
				E.push_back(Etxt);
			}else if(elm["type"].asString() == "selector"){
				SchemaElement * Esel = new SchemaElementSelector(addr, elm);
				E.push_back(Esel);
			}else if(elm["type"].asString() == "color"){
				SchemaElement * Ecol = new SchemaElementColor(addr, elm);
				E.push_back(Ecol);
			}else if(elm["type"].asString() == "tuple"){
				SchemaElement * Etup = new SchemaElementTuple(addr, elm);
				E.push_back(Etup);
			}else{
				std::cerr << "Unknown element type: " << elm["type"].asString() << "\n";
				exit(EXIT_FAILURE);
				return;
			}
		}
	}else{
		std::cerr << "SchemaTupleElement Missing elms\n";
		exit(EXIT_FAILURE);
		return;
	}
}

SchemaElementTuple::~SchemaElementTuple(){
	for(SchemaElement *elm : E){
		delete elm;
	}
}

void SchemaElementTuple::verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators)
{
	for(SchemaElement *elm : E){
		if(in.isMember(elmid)){
			elm->verifyElement(in[elmid], err, actutators);
		}else{
			Json::Value container;
			elm->verifyElement(container, err, actutators);
			in[elmid] = container;
		}
	}
}


