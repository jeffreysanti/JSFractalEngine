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

class SchemaElement {
public:
	virtual bool verifyElement(Json::Value &elm, Json::Value &in, std::string &err, std::set<std::string> &actutators) = 0;

	template<class T> void numericConstraints(Json::Value &elm, bool &max, bool &min, T &maxv, T &minv, bool &zero);
	bool verifyPresence(Json::Value &elm, Json::Value &in, std::string &err);
	bool isPresent(Json::Value &elm, Json::Value &in);
	bool allowNull(Json::Value &elm);
};

class SchemaElementIntegral : public SchemaElement {
public:
	virtual bool verifyElement(Json::Value &elm, Json::Value &in, std::string &err, std::set<std::string> &actutators);
};
class SchemaElementText : public SchemaElement {
public:
	virtual bool verifyElement(Json::Value &elm, Json::Value &in, std::string &err, std::set<std::string> &actutators);
};

#endif /* SCHEMAELEMENT_H_ */
