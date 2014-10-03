/*
 * SchemaManager.h
 *
 *  Created on: Sep 15, 2014
 *      Author: jeffrey
 */

#ifndef SCHEMAMANAGER_H_
#define SCHEMAMANAGER_H_

#include "SchemaElement.h"

/*
enum ElmType{
	ELM_VOID,
	ELM_STRING,
	ELM_INTEGER
};
struct NumericConstraintIntegral{
	bool minimized, maximized;
	long long min;
	long long max;
};

struct NumericConstraintReal{
	bool minimized, maximized;
	long double min;
	long double max;
};

class Element{
public:
	std::string name;
	bool allowNull;

	virtual inline const ElmType getType(){
		return ELM_VOID;
	}
};
class ElementText : public Element{
public:
	NumericConstraintIntegral NC;
	virtual inline const ElmType getType(){
		return ELM_STRING;
	}
};
class ElementInteger : public Element{
public:
	NumericConstraintIntegral NC;
	virtual inline const ElmType getType(){
		return ELM_INTEGER;
	}
};


class GroupConstraint{

};
*/


class SchemaManager {
public:

	void initialize();

	std::string validateParamaters(Json::Value paramRoot);

	static SchemaManager *getSingleton();

private:
	SchemaManager();
	~SchemaManager();

	//Element parseElement(Json::Value elm);

	Json::Value root;

	std::map <std::string, Json::Value> G;

	void findConditionlessGroups(std::deque<std::string> &lst);

	static SchemaManager singleton;
};

#endif /* SCHEMAMANAGER_H_ */
