/*
 * DBSystem.h
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#ifndef DBSYSTEM_H_
#define DBSYSTEM_H_

#include "sqlite3.h"
#include "UserManager.h"
#include <sstream>
#include <string>
#include <mutex>
#include "ClientCommunicator.h"
#include "ParamsFile.h"

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
		manualQueue = true;
	}

	int jobID;
	int userID;
	std::string name;
	std::string author;
	FractalDBState status;
	bool manualQueue;
};

struct FractalContainer{
	FractalMeta m;
	ParamsFile *p;
};


class DBManager {
public:

	//Authentication auth;
	void initialize();

	std::string getJobSearchResult(std::string query);

	FractalMeta getFractal(int jid);
	void insertFractal(FractalMeta f);
	void updateFractal(FractalMeta f);

	void fillMDUDRequest(int jid, char **sz, int &len);
	void fillRCTXRequest(int jid, char **sz, int &len);

	unsigned int submitJob(FractalMeta meta, ParamsFile *p);
	std::vector<FractalContainer> fetchSubmittedJobs();

	static DBManager *getSingleton();

private:

	DBManager();
	virtual ~DBManager();

	void executeSql(std::string cmd);
	void dbVerify(int res);

	static DBManager singleton;


	sqlite3 *s;
	unsigned int lastID;

	std::mutex mtx;

	std::vector<FractalContainer> Submitted;

};

template<typename TYP>
inline std::string concat(std::string s, TYP i)
{
	std::stringstream ss;
	ss << s << i;
	return ss.str();
}

#endif /* DBSYSTEM_H_ */
