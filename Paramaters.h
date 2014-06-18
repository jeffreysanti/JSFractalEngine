/*
 * Paramaters.h
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#ifndef PARAMATERS_H_
#define PARAMATERS_H_

#include <map>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

class Paramaters {
public:
	Paramaters();
	virtual ~Paramaters();

	void setValue(std::string key, std::string val);
	std::string getValue(std::string key, std::string defValue="");

	void dumpSet();
	std::string asString();

	bool writeToFile(std::string flName);
	bool writeToFile();
	void loadFromString(std::string str);
	bool loadFromFile(std::string flName);

	void setID(int i);
	int getID();

	void setUID(int i);
	int getUID();

private:
	std::map<std::string, std::string> P;
	int id;
	int uid;
	std::string lastFileName;
};

#endif /* PARAMATERS_H_ */
