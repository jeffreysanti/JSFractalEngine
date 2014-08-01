/*
 * Authentication.h
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#ifndef AUTHENTICATION_H_
#define AUTHENTICATION_H_

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

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


class Authentication {
public:
	Authentication(std::string fl);
	virtual ~Authentication();

	User getById(unsigned int id);
	User getByName(std::string name);

	std::string dumpAuthDetails();

private:

	std::vector<User> U;
};

#endif /* AUTHENTICATION_H_ */
