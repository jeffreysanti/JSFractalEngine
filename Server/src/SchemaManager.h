/*
 * SchemaManager.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef SCHEMAMANAGER_H_
#define SCHEMAMANAGER_H_

#include "DirectoryManager.h"


class SchemaManager {
public:

	void initialize();


	static SchemaManager *getSingleton();

private:
	SchemaManager();
	~SchemaManager();

	Json::Value root;

	static SchemaManager singleton;
};

#endif /* SCHEMAMANAGER_H_ */
