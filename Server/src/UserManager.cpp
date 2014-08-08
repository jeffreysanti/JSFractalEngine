/*
 * Authentication.cpp
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#include "UserManager.h"

UserManager UserManager::singleton;

UserManager::UserManager(){
}

UserManager::~UserManager() {
}

void UserManager::initialize() {

	Json::Value &root = DirectoryManager::getSingleton()->getConfigRoot();
	if(!root.isMember("users") || !root["users"].isArray() || root["users"].empty()){
		std::cerr << "Configuration 'users' invalid [needs at least one child] or non-existent!\n";
		exit(EXIT_FAILURE);
		return;
	}
	for(auto elm : root["users"]){
		if(!elm.isMember("name") || !elm.isMember("pass") || !elm.isMember("admin") || !elm.isMember("automated")){
			std::cerr << "Each user must have 'name', 'pass', 'admin', 'automated' elements!\n";
			exit(EXIT_FAILURE);
			return;
		}
		if(!elm.isMember("id") || !elm["id"].isInt()){
			std::cerr << "Each user must have an integer id element!\n";
			exit(EXIT_FAILURE);
			return;
		}
		User u;
		u.admin = elm["admin"].asBool();
		u.daemon = elm["automated"].asBool();
		u.name = elm["name"].asString();
		u.pass = elm["pass"].asString();
		u.id = elm["id"].asInt();
		U.push_back(u);
	}
	std::cout << "-----AUTHENTICATION OPTIONS: -----\n" << dumpAuthDetails() << "==================\n";
}

UserManager *UserManager::getSingleton(){
	return &singleton;
}



User UserManager::getById(unsigned int id)
{
	for(int i=0; i<U.size(); i++){
		if(U[i].id == id)
			return U[i];
	}
	return User();
}

User UserManager::getByName(std::string name)
{
	for(int i=0; i<U.size(); i++){
		if(U[i].name == name)
			return U[i];
	}
	return User();
}

std::string UserManager::dumpAuthDetails(){
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

User UserManager::getNoSocketsAccount()
{
	User u;
	u.admin = true;
	u.daemon = false;
	u.name = "--nosockets--";
	u.pass = "-";
	u.id = 199991;
	return u;
}






