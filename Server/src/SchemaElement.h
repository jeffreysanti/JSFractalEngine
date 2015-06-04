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
#include <complex>
#include "muParserC/mucParser.h"

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

	inline void overrideElementID(std::string eid) {elmid = eid;}

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
class SchemaElementComplex : public SchemaElement {
public:
	SchemaElementComplex(std::string grpAddr, Json::Value &schema);
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	std::string defVal;
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
	virtual ~SchemaElementSelector();
	virtual void verifyElement(Json::Value &in, std::string &err, std::vector<SchemaActuator> &actutators);
protected:
	std::map<std::string, std::vector<SchemaActuator>> O;
	std::map<std::string, Json::Value> SubElms;
	std::string defVal;
	SchemaElement *E;
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


inline std::complex<double> getComplexValueFromString(std::string val){
	try{
		muc::Parser p;
		p.SetExpr(val);
		std::complex<double> z = p.Eval();
		return z;
	}catch (muc::Parser::exception_type &e)
	{
		return std::complex<double>(0,0); // if failed
	}
}


#endif /* SCHEMAELEMENT_H_ */
