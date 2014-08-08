/*
 * Authentication.h
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#ifndef USERMAN_H_
#define USERMAN_H_

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include "DirectoryManager.h"

struct User{

	User(){
		id = -1;
		name = "";
		pass = "";
		admin = false;
		daemon = false;
	}

	int id;
	std::string name;
	std::string pass;
	bool admin;
	bool daemon;
};


class UserManager {
public:

	void initialize();

	User getById(unsigned int id);
	User getByName(std::string name);

	std::string dumpAuthDetails();

	User getNoSocketsAccount();

	static UserManager *getSingleton();

private:

	UserManager();
	~UserManager();

	static UserManager singleton;

	std::vector<User> U;
};

#endif /* USERMAN_H_ */
