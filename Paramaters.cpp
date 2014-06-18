/*
 * Paramaters.cpp
 *
 *  Created on: Apr 26, 2014
 *      Author: jeffrey
 */

#include "Paramaters.h"

Paramaters::Paramaters() {
	// TODO Auto-generated constructor stub
	id = -1;
	lastFileName = "./tmp";
}

Paramaters::~Paramaters() {
	// TODO Auto-generated destructor stub
}

void Paramaters::setID(int i)
{
	if(id != -1) // cannot be changed
		return;
	id = i;
}

int Paramaters::getID()
{
	return id;
}

void Paramaters::setUID(int i)
{
	if(uid != -1) // cannot be changed
		return;
	uid = i;
}

int Paramaters::getUID()
{
	return uid;
}

void Paramaters::setValue(std::string key, std::string val)
{
	P[key] = val;
}

std::string Paramaters::getValue(std::string key, std::string defValue)
{
	std::string ret = P[key];
	if(ret == ""){
		ret = defValue;
		P[key] = ret;
	}
	return ret;
}

void Paramaters::dumpSet()
{
	for(std::map<std::string, std::string>::iterator it=P.begin(); it != P.end(); it++)
	{
		std::cout << (*it).first << ":" << (*it).second << "\n";
	}
}

std::string Paramaters::asString()
{
	std::string tmp = "";
	for(std::map<std::string, std::string>::iterator it=P.begin(); it != P.end(); it++)
	{
		tmp += (*it).first + ":" + (*it).second + "\n";
	}
	return tmp;
}

bool Paramaters::writeToFile(std::string flName)
{
	lastFileName = flName;
	std::fstream fp;
	fp.open(flName.c_str(), std::ios::out);
	if(fp.is_open()){
		for(std::map<std::string, std::string>::iterator it=P.begin(); it != P.end(); it++)
		{
			fp << (*it).first << ":" << (*it).second << "\n";
		}
		fp.close();
		return true;
	}else{
		std::cout << "Error Opening " << flName << " dumping instead:\n";
		dumpSet();
		return false;
	}
}

bool Paramaters::writeToFile()
{
	std::fstream fp;
	fp.open(lastFileName.c_str(), std::ios::out);
	if(fp.is_open()){
		for(std::map<std::string, std::string>::iterator it=P.begin(); it != P.end(); it++)
		{
			fp << (*it).first << ":" << (*it).second << "\n";
		}
		fp.close();
		return true;
	}else{
		std::cout << "Error Opening " << lastFileName << " [lastFileName] dumping instead:\n";
		dumpSet();
		return false;
	}
}

void Paramaters::loadFromString(std::string str){
	std::string line = "";
	while(str.find('\r') != str.npos)
		str = str.replace(str.find('\r'), 1, "\n");
	while(str.find('\n') != str.npos){
		line = str.substr(0, str.find('\n'));
		str = str.substr(str.find('\n')+1);

		size_t index = line.find(':');
		if(index == line.npos)
			continue;
		std::string param = line.substr(0, index);
		std::string val = line.substr(index+1);
		setValue(param, val);
	}
	line = str;
	size_t index = line.find(':');
	if(index == line.npos)
		return;
	std::string param = line.substr(0, index);
	std::string val = line.substr(index+1);
	setValue(param, val);
}

bool Paramaters::loadFromFile(std::string flName)
{
	lastFileName = flName;
	std::fstream fp;
	fp.open(flName.c_str(), std::ios::in);
	if(fp.is_open()){
		std::string line;
		while(getline(fp,line))
		{
			size_t index = line.find(':');
			if(index == line.npos)
				continue;
			std::string param = line.substr(0, index);
			std::string val = line.substr(index+1);
			setValue(param, val);
		}
		return true;
	}else{
		return false;
	}
}




