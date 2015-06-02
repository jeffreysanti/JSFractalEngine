/*
 * SchemaManager.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef SCHEMAMANAGER_H_
#define SCHEMAMANAGER_H_

#include "SchemaElement.h"

enum SchemaValType{
	SVT_COLOR,
	SVT_INT,
	SVT_REAL,
	SVT_TEXT,
	SVT_COMPLEX,
	SVT_SELECTOR_NO_AFFECT,

	SVT_ERR
};

struct SchemaValTypeJsonObj{

	SchemaValTypeJsonObj(SchemaValType t, Json::Value j, Json::Value *p)
		: param(p)
	{
		vt = t;
		elm = j;
	}

	SchemaValType vt;
	Json::Value elm;
	Json::Value *param;
};

class SchemaManager {
public:

	void initialize();

	std::string validateParamaters(Json::Value &paramRoot);

	bool validateAnimationParam(Json::Value &keyframeRoot, int frameCount, std::string &err);
	SchemaValTypeJsonObj getParamAddrType(std::string addr, Json::Value *paramRoot);

	Json::Value *getParamByAddr(Json::Value *paramRoot, std::string addr);

	static SchemaManager *getSingleton();

private:
	SchemaManager();
	~SchemaManager();

	Json::Value inital_root;
	std::map <std::string, Json::Value> inital_G;

	void findConditionlessGroups(std::deque<std::string> &lst);

	static SchemaManager singleton;
};

#endif /* SCHEMAMANAGER_H_ */
