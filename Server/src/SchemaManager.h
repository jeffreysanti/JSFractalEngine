/*
 * SchemaManager.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef SCHEMAMANAGER_H_
#define SCHEMAMANAGER_H_

#include "SchemaElement.h"


class SchemaManager {
public:

	void initialize();

	std::string validateParamaters(Json::Value &paramRoot);

	static SchemaManager *getSingleton();

private:
	SchemaManager();
	~SchemaManager();

	Json::Value root;

	std::map <std::string, Json::Value> G;

	void findConditionlessGroups(std::deque<std::string> &lst);

	static SchemaManager singleton;
};

#endif /* SCHEMAMANAGER_H_ */
