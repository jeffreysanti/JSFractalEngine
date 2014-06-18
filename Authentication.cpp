/*
 * Authentication.cpp
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#include "Authentication.h"

Authentication::Authentication(std::string fl) {
	std::fstream fp;
	fp.open(fl.c_str(), std::ios::in);
	if(fp.is_open()){
		std::string line;
		while(getline(fp,line))
		{
			if(line.length() < 2)
				continue;
			User u;

			if(line.at(0) == '*'){
				u.admin = true;
				line = line.substr(1);
			}

			if(line.at(0) == '#'){ // daemon client -> send to auto queue
				u.daemon = true;
				line = line.substr(1);
			}

			u.id = atoi(line.substr(0, line.find(":")).c_str());
			line = line.substr(line.find(":")+1);
			u.name = line.substr(0, line.find("/"));
			u.pass = line.substr(line.find("/") + 1);
			U.push_back(u);
		}
	}else{
		std::cerr << "Error: Passwd File Missing:" << fl << "\n";
	}
}

Authentication::~Authentication() {
}

User Authentication::getById(unsigned int id)
{
	for(int i=0; i<U.size(); i++){
		if(U[i].id == id)
			return U[i];
	}
	return User();
}

User Authentication::getByName(std::string name)
{
	for(int i=0; i<U.size(); i++){
		if(U[i].name == name)
			return U[i];
	}
	return User();
}

std::string Authentication::dumpAuthDetails(){
	std::string tmp = "";
	for(int i=0; i<U.size(); i++){
		tmp += "USER: '" + U[i].name + "' PASSWORD: '" + U[i].pass + "'\n";
		if(U[i].admin)
			tmp += "   -> IS ADMIN\n";
		if(U[i].daemon)
			tmp += "   -> IS DAEMON [LOWER PRIORITY AUTO QUEUE]\n";
	}
	return tmp;
}






