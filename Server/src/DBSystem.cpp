/*
 * DBSystem.cpp
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#include "DBSystem.h"

DBSystem::DBSystem(std::string dbDir) : auth(dbDir + "/passwd") {
	int ret = sqlite3_open(std::string(dbDir+"db").c_str(), &s);

	if(ret){
		std::cerr << "DB Open Failure" << sqlite3_errmsg(s);
		exit(0);
	}

	// build db structure
	executeSql("CREATE TABLE IF NOT EXISTS fract (" \
			"ID		INTEGER NOT NULL," \
			"UID	INTEGER NOT NULL," \
			"NAME	TEXT NOT NULL," \
			"AUTHOR	TEXT NOT NULL," \
			"STATUS	INTEGER NOT NULL )");

	sqlite3_stmt *stmt;

	std::string query = "SELECT ID FROM fract ORDER BY ID DESC LIMIT 1";
	ret = sqlite3_prepare(s, query.c_str(), query.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	dbVerify(ret);
	if(ret != SQLITE_ROW){
		lastID = 0;
	}else{
		lastID = sqlite3_column_int(stmt, 0);
	}

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);

	// these were prematurely canceled with either crash or force quit
	executeSql(concat("UPDATE fract SET STATUS=",FDBS_ERR)+concat(" WHERE STATUS=",FDBS_RENDERING)+
			concat(" OR STATUS=",FDBS_QUEUED));
}

DBSystem::~DBSystem() {
	sqlite3_close(s);
}

void DBSystem::dbVerify(int res)
{
	if(res != SQLITE_OK && res != SQLITE_DONE && res != SQLITE_ROW){
		std::cerr << "DB Error: " << sqlite3_errmsg(s);
		exit(0);
	}
}

void DBSystem::executeSql(std::string cmd)
{
	sqlite3_stmt *stmt;

	int ret = sqlite3_prepare(s, cmd.c_str(), cmd.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	dbVerify(ret);

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);
}

std::string DBSystem::getJobSearchResult(std::string query)
{
	if(query.find('|') == query.npos)
		return "";
	int limitSz = atoi(query.substr(0, query.find('|')).c_str());
	query = query.substr(query.find('|')+1);

	std::string q;
	sqlite3_stmt *stmt;
	int ret;

	if(query == ""){
		q = concat("SELECT ID FROM fract ORDER BY STATUS ASC, ID DESC LIMIT ", limitSz);
		ret = sqlite3_prepare(s, q.c_str(), q.length(), &stmt, NULL);
		dbVerify(ret);
	}else{
		query = "%"+query+"%"; // match partial
		q = concat("SELECT ID FROM fract WHERE ID LIKE ? OR NAME LIKE ? OR AUTHOR LIKE ? " \
				"ORDER BY STATUS ASC, ID DESC LIMIT ", limitSz);
		ret = sqlite3_prepare(s, q.c_str(), q.length(), &stmt, NULL);
		dbVerify(ret);

		for(int i=1; i<=3; i++){
			ret = sqlite3_bind_text(stmt, i, query.c_str(), query.length(), SQLITE_STATIC);
			dbVerify(ret);
		}
	}

	std::string out = "";
	ret = sqlite3_step(stmt);
	while(ret == SQLITE_ROW)
	{
		out += concat(",",sqlite3_column_int(stmt, 0));

		ret = sqlite3_step(stmt);
	}
	dbVerify(ret);

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);

	if(out.length() > 0)
		return out.substr(1);
	return "";
}

FractalMeta DBSystem::getFractal(int jid){
	std::string q;
	sqlite3_stmt *stmt;
	int ret;
	FractalMeta fract;

	q = "SELECT UID, NAME, AUTHOR, STATUS FROM fract WHERE ID = ?";
	ret = sqlite3_prepare(s, q.c_str(), q.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 1, jid);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	if(ret == SQLITE_ROW){
		fract.jobID = jid;
		fract.userID = sqlite3_column_int(stmt, 0);
		fract.name = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
		fract.author = std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
		fract.status = (FractalDBState)sqlite3_column_int(stmt, 3);
	}

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);

	return fract;
}

void DBSystem::insertFractal(FractalMeta f){
	std::string q;
	sqlite3_stmt *stmt;
	int ret;
	FractalMeta fract;

	q = "INSERT INTO fract (ID, UID, NAME, AUTHOR, STATUS) VALUES (?,?,?,?,?)";
	ret = sqlite3_prepare(s, q.c_str(), q.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 1, f.jobID);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 2, f.userID);
		dbVerify(ret);

	ret = sqlite3_bind_text(stmt, 3, f.name.c_str(), f.name.length(), SQLITE_STATIC);
	dbVerify(ret);

	ret = sqlite3_bind_text(stmt, 4, f.author.c_str(), f.author.length(), SQLITE_STATIC);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 5, (int)f.status);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	dbVerify(ret);

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);
}

void DBSystem::updateFractal(FractalMeta f)
{
	std::string q;
	sqlite3_stmt *stmt;
	int ret;
	FractalMeta fract;

	q = "UPDATE fract SET NAME=?, AUTHOR=?, STATUS=? WHERE ID=?";
	ret = sqlite3_prepare(s, q.c_str(), q.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_bind_text(stmt, 1, f.name.c_str(), f.name.length(), SQLITE_STATIC);
	dbVerify(ret);

	ret = sqlite3_bind_text(stmt, 2, f.author.c_str(), f.author.length(), SQLITE_STATIC);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 3, (int)f.status);
	dbVerify(ret);

	ret = sqlite3_bind_int(stmt, 4, f.jobID);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	dbVerify(ret);

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);
}

unsigned int DBSystem::getNextID(){
	lastID ++;
	return lastID;
}





