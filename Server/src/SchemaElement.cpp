/*
 * SchemaElement.cpp
 *
 *  Created on: Oct 3, 2014
 *      Author: jeffrey
 */

#include "SchemaElement.h"

bool SchemaElement::allowNull(Json::Value &elm)
{
	if(elm.isMember("allowNull") && !elm["allowNull"].asBool()){
		return false;
	}else{
		return true;
	}
}

template<class T> void SchemaElement::numericConstraints(Json::Value &elm, bool &max, bool &min, T &maxv, T &minv, bool &zero)
{
	zero = allowNull(elm);
	min = false;
	max = false;
	if(elm.isMember("min")){
		min = true;
		minv = elm["min"].asDouble();
	}
	if(elm.isMember("min")){
		max = true;
		maxv = elm["max"].asDouble();
	}
}

bool SchemaElement::isPresent(Json::Value &elm, Json::Value &in)
{
	if(!in.isMember(elm["id"].asString())){
		return false;
	}
	return true;
}

bool SchemaElement::verifyPresence(Json::Value &elm, Json::Value &in, std::string &err)
{
	if(!isPresent(elm, in)){
		err = "Element " + elm["id"].asString() + " value illegally null or zero";
		return false;
	}
	return true;
}


bool SchemaElementIntegral::verifyElement(Json::Value &elm, Json::Value &in, std::string &err, std::set<std::string> &actutators)
{
	bool minimized, maximized, zero;
	long long min;
	long long max;
	numericConstraints(elm, maximized, minimized, max, min, zero);
	if(!zero && !verifyPresence(elm, in, err)){
		return false;
	}else if(!isPresent(elm, in)){ // default value
		if(elm.isMember("default")) in[elm["id"].asString()] = elm["default"].asLargestInt();
		else 						in[elm["id"].asString()] = 1;
		return true;
	}

	// verify input
	long long val = in[elm["id"].asString()].asLargestInt();
	if(maximized && val > max){
		err = "Maximum Constraint Violated on " + elm["id"].asString();
		return false;
	}
	if(maximized && val < min){
		err = "Minimum Constraint Violated on " + elm["id"].asString();
		return false;
	}
	return true;
}

bool SchemaElementText::verifyElement(Json::Value &elm, Json::Value &in, std::string &err, std::set<std::string> &actutators)
{
	if(!allowNull(elm) && !verifyPresence(elm, in, err)){
		return false;
	}else if(!isPresent(elm, in)){
		if(elm.isMember("default")) in[elm["id"].asString()] = elm["default"].asString();
		else 						in[elm["id"].asString()] = " ";
		return true;
	}
	return true;
}

