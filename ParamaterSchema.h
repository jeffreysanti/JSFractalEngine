/*
 * ParamaterSchema.h
 *
 *  Created on: May 9, 2014
 *      Author: jeffrey
 */

#ifndef PARAMATERSCHEMA_H_
#define PARAMATERSCHEMA_H_

#include "Paramaters.h"
#include <map>

struct Color;

enum PARAM_TYPE{
	PARAM_TYPE_STRING = 0,
	PARAM_TYPE_WHOLE = 1,
	PARAM_TYPE_REAL = 2,
	PARAM_TYPE_BOOL = 3,
	PARAM_TYPE_COLOR = 4
};

enum EXTERMA_TYPE{
	EXT_IGNORE = 0,
	EXT_INCLUSIVE = 1,
	EXT_EXCLUSIVE = 2
};

struct ParamConstraints{
	std::string name;

	PARAM_TYPE type;
	bool zeroNullOkay; // can this be zero (if numeric) or empty string
	std::string defValue;

	bool setOf;

	// only for numeric types
	double min; EXTERMA_TYPE emin;
	double max; EXTERMA_TYPE emax;

	int setMin;
	int setMax;
};


class ParamaterSchema {
public:
	ParamaterSchema(std::string schemaFile);
	virtual ~ParamaterSchema();

	bool isErr();
	std::string clearErrors();

	std::string getString(Paramaters &p, std::string param);
	int getInt(Paramaters &p, std::string param);
	double getDouble(Paramaters &p, std::string param);
	bool getBool(Paramaters &p, std::string param);
	Color getColor(Paramaters &p, std::string param);

protected:
	ParamConstraints lineParse(std::string line);
	std::map<std::string, ParamConstraints> C;
	std::string errors;
};

#endif /* PARAMATERSCHEMA_H_ */
