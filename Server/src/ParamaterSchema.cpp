/*
 * ParamaterSchema.cpp
 *
 *  Created on: May 9, 2014
 *      Author: jeffrey
 */

#include "ParamaterSchema.h"
#include "ColorPalette.h"
#include "DirectoryManager.h"

ParamaterSchema::ParamaterSchema(std::string schemaFile) {

	// relative to config path
	schemaFile = DirectoryManager::getSingleton()->getRootDirectory() + schemaFile;

	std::fstream fp;
	fp.open(schemaFile.c_str(), std::ios::in);
	if(fp.is_open()){
		std::string line;
		while(getline(fp,line))
		{
			if(line.length() < 2)
				continue;
			ParamConstraints c = lineParse(line);
			C[c.name] = c;
		}
	}else{
		std::cerr << "Error: Schema File Missing:" << schemaFile << "\n";
	}

}

ParamaterSchema::~ParamaterSchema() {
	// TODO Auto-generated destructor stub
}

// Format of line as follows:
// <paramName>:<VariableType>/<AllowNull? Y/N>(range)|<defValue>
// Variable Types: S for strings, R for real values, W for whole number, B for boolean, C for color
// range is in set format [ ( inclusive / exclusive ) ]
// a question mark in paramName indicates a set-of followed by min;max # of elements
// if no min or max just leave empty ie: "colorSet?;" or "colorSet?1;"
ParamConstraints ParamaterSchema::lineParse(std::string line)
{
	ParamConstraints c;
	c.setOf = false;
	c.min = 0; c.emin = EXT_IGNORE;
	c.max = 0; c.emax = EXT_IGNORE;
	c.setOf = false;
	c.setMin = -1;
	c.setMax = -1;

	c.name = line.substr(0, line.find(':'));
	if(line.find('?') != line.npos){
		c.setOf = true; // TODO: Implement Set min/max
	}
	line = line.substr(line.find(':')+1);
	char type = line[0];
	c.type = PARAM_TYPE_STRING;
	if(type == 'R') c.type = PARAM_TYPE_REAL;
	if(type == 'W') c.type = PARAM_TYPE_WHOLE;
	if(type == 'B') c.type = PARAM_TYPE_BOOL;
	if(type == 'C') c.type = PARAM_TYPE_COLOR;

	line = line.substr(line.find('/')+1);
	type = line[0];
	if(type == 'Y') c.zeroNullOkay = true;
	else c.zeroNullOkay = false;

	line = line.substr(1);
	if(line.at(0) == '(' || line.at(0) == '['){ // range exclusive
		c.emin = EXT_EXCLUSIVE;
		if(line.at(0) == '[')
			c.emin = EXT_INCLUSIVE;

		line = line.substr(1);
		c.min = strtod(line.substr(0, line.find(',')).c_str(), NULL);
		line = line.substr(line.find(',') + 1);
		if(line.find(')') != line.npos){
			c.emax = EXT_EXCLUSIVE;
			c.max = strtod(line.substr(0, line.find(')')).c_str(), NULL);
			line = line.substr(line.find(')')+1);
		}else{
			c.emax = EXT_INCLUSIVE;
			c.max = strtod(line.substr(0, line.find(']')).c_str(), NULL);
			line = line.substr(line.find(']')+1);
		}
	}

	line = line.substr(line.find('|')+1);
	c.defValue = line;
	return c;
}

bool ParamaterSchema::isErr()
{
	return errors.length() > 0;
}

std::string ParamaterSchema::clearErrors()
{
	std::string ret = errors;
	errors = "";
	return ret;
}

std::string ParamaterSchema::getString(Paramaters &p, std::string param)
{
	std::string pname = param;
	if(pname.find('$') != pname.npos){
		pname = pname.substr(0, pname.find('$')) + "?";
	}
	ParamConstraints c = C[pname];
	if(c.name != pname){
		errors += "Unknown Parameter: " + pname + "\n";
	}
	std::string val = p.getValue(param, c.defValue);
	if(!c.zeroNullOkay && val.length() < 1){
		errors += "Parameter " + pname + " Illegally zero length\n";
		return c.defValue;
	}
	return val;
}

int ParamaterSchema::getInt(Paramaters &p, std::string param)
{
	std::string pname = param;
	if(pname.find('$') != pname.npos){
		pname = pname.substr(0, pname.find('$')) + "?";
	}
	ParamConstraints c = C[pname];
	if(c.name != pname){
		errors += "Unknown Parameter: " + pname + "\n";
	}
	int val = atoi(p.getValue(param, c.defValue).c_str());
	if(!c.zeroNullOkay && val == 0){
		errors += "Parameter " + pname + " Illegally zero\n";
		return atoi(c.defValue.c_str());
	}
	if((c.emin == EXT_INCLUSIVE && val < c.min) ||
			(c.emin == EXT_EXCLUSIVE && val <= c.min)){
		errors += "Parameter " + pname + " minimum constraint violated!\n";
		return atoi(c.defValue.c_str());
	}
	if((c.emax == EXT_INCLUSIVE && val > c.max) ||
			(c.emax == EXT_EXCLUSIVE && val >= c.max)){
		errors += "Parameter " + pname + " maximum constraint violated!\n";
		return atoi(c.defValue.c_str());
	}
	return val;
}

double ParamaterSchema::getDouble(Paramaters &p, std::string param)
{
	std::string pname = param;
	if(pname.find('$') != pname.npos){
		pname = pname.substr(0, pname.find('$')) + "?";
	}
	ParamConstraints c = C[pname];
	if(c.name != pname){
		errors += "Unknown Parameter: " + pname + "\n";
	}
	double val = strtod(p.getValue(param, c.defValue).c_str(), NULL);
	if(!c.zeroNullOkay && val == 0){
		errors += "Parameter " + pname + " Illegally zero\n";
		return strtod(c.defValue.c_str(), NULL);
	}
	if((c.emin == EXT_INCLUSIVE && val < c.min) ||
			(c.emin == EXT_EXCLUSIVE && val <= c.min)){
		errors += "Parameter " + pname + " minimum constraint violated!\n";
		return atoi(c.defValue.c_str());
	}
	if((c.emax == EXT_INCLUSIVE && val > c.max) ||
			(c.emax == EXT_EXCLUSIVE && val >= c.max)){
		errors += "Parameter " + pname + " maximum constraint violated!\n";
		return atoi(c.defValue.c_str());
	}
	return val;
}

bool ParamaterSchema::getBool(Paramaters &p, std::string param)
{
	std::string pname = param;
	if(pname.find('$') != pname.npos){
		pname = pname.substr(0, pname.find('$')) + "?";
	}
	ParamConstraints c = C[pname];
	if(c.name != pname){
		errors += "Unknown Parameter: " + pname + "\n";
	}
	std::string val = p.getValue(param, c.defValue);
	if(val == "T" || val == "Y" || val == "YES")
		return true;
	return false;
}

Color ParamaterSchema::getColor(Paramaters &p, std::string param)
{
	std::string pname = param;
	if(pname.find('$') != pname.npos){
		pname = pname.substr(0, pname.find('$')) + "?";
	}

	ParamConstraints c = C[pname];
	if(c.name != pname){
		errors += "Unknown Parameter: " + pname + "\n";
	}
	Color val = ColorPalette::fromParam(p.getValue(param, c.defValue), Color(0,0,0));
	return val;
}

