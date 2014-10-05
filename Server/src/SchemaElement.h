/*
 * SchemaElement.h
 *
 *  Created on: Oct 3, 2014
 *      Author: jeffrey
 */

#ifndef SCHEMAELEMENT_H_
#define SCHEMAELEMENT_H_

#include "DirectoryManager.h"
#include <queue>
#include <set>

struct SchemaActuator{
	std::string action; // either "hide" or "show"
	std::string match;
};

class SchemaElement {
public:
	SchemaElement(std::string grpAddr, Json::Value &schema);
	virtual ~SchemaElement();

	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators) = 0;

	template<class T> void numericConstraints(bool &max, bool &min, T &maxv, T &minv, bool &zero);
	bool isPresent(Json::Value &in);
	bool allowNull();

	inline std::string getElementID() {return elmid;}

protected:
	std::string addr;
	std::string elmid;
	Json::Value &schm;
};

class SchemaElementIntegral : public SchemaElement {
public:
	SchemaElementIntegral(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	bool allowNullZero;
	bool maximized, minimized;
	long long min, max;
	long long defVal;
};
class SchemaElementReal : public SchemaElement {
public:
	SchemaElementReal(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	bool allowNullZero;
	bool maximized, minimized;
	long double min, max;
	long double defVal;
};
class SchemaElementText : public SchemaElement {
public:
	SchemaElementText(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	bool allowNullZero;
	bool maximized, minimized;
	unsigned int min, max;
	std::string defVal;
};
class SchemaElementSelector : public SchemaElement {
public:
	SchemaElementSelector(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	std::map<std::string, std::vector<SchemaActuator>> O;
	std::string defVal;
};
class SchemaElementColor : public SchemaElement {
public:
	SchemaElementColor(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	unsigned char defR, defG, defB;
};
class SchemaElementTuple : public SchemaElement {
public:
	SchemaElementTuple(std::string grpAddr, Json::Value &schema);
	virtual ~SchemaElementTuple();
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	std::vector<SchemaElement*> E;
};
class SchemaElementArray : public SchemaElement {
public:
	SchemaElementArray(std::string grpAddr, Json::Value &schema);
	virtual ~SchemaElementArray();
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	bool maximized, minimized;
	long long min, max;
	SchemaElement *E;
};


#endif /* SCHEMAELEMENT_H_ */
