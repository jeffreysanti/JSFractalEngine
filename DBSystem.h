/*
 * DBSystem.h
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#ifndef DBSYSTEM_H_
#define DBSYSTEM_H_

#include "sqlite3.h"
#include "Authentication.h"
#include <sstream>
#include <string>

enum FractalDBState{
	FDBS_RENDERING = 0,
	FDBS_COMPLETE = 10,
	FDBS_QUEUED = 20,
	FDBS_CANCEL = 30,
	FDBS_TIMEOUT = 40,
	FDBS_ERR = 50
};

struct FractalMeta{

	FractalMeta(){
		jobID = -1;
		userID = -1;
		status = FDBS_ERR;
	}

	int jobID;
	int userID;
	std::string name;
	std::string author;
	FractalDBState status;
};


class DBSystem {
public:
	DBSystem(std::string dbDir);
	virtual ~DBSystem();

	Authentication auth;

	std::string getJobSearchResult(std::string query);

	FractalMeta getFractal(int jid);
	void insertFractal(FractalMeta f);
	void updateFractal(FractalMeta f);

	unsigned int getNextID();

private:

	void executeSql(std::string cmd);
	void dbVerify(int res);


	sqlite3 *s;
	unsigned int lastID;

};

template<typename TYP>
inline std::string concat(std::string s, TYP i)
{
	std::stringstream ss;
	ss << s << i;
	return ss.str();
}

#endif /* DBSYSTEM_H_ */
