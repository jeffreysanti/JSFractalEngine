/*
 * DBSystem.cpp
 *
 *  Created on: May 28, 2014
 *      Author: jeffrey
 */

#include "DBManager.h"

DBManager DBManager::singleton;

DBManager::DBManager() {
	s = nullptr;
}

DBManager::~DBManager() {
	sqlite3_close(s);
}

DBManager *DBManager::getSingleton(){
	return &singleton;
}

void DBManager::initialize(){

	if(s != nullptr){
		std::cerr << "DBManager::initialize called multiple times!\n";
		exit(EXIT_FAILURE);
		return;
	}

	std::string dbPath = DirectoryManager::getSingleton()->getRootDirectory() + "db";
	int ret = sqlite3_open(dbPath.c_str(), &s);

	if(ret){
		std::cerr << "DB Open Failure" << sqlite3_errmsg(s) << "\n";
		exit(EXIT_FAILURE);
		return;
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


void DBManager::dbVerify(int res)
{
	if(res != SQLITE_OK && res != SQLITE_DONE && res != SQLITE_ROW){
		std::cerr << "DB Error: " << sqlite3_errmsg(s);
		exit(0);
	}
}

void DBManager::executeSql(std::string cmd)
{
	sqlite3_stmt *stmt;

	int ret = sqlite3_prepare(s, cmd.c_str(), cmd.length(), &stmt, NULL);
	dbVerify(ret);

	ret = sqlite3_step(stmt);
	dbVerify(ret);

	ret = sqlite3_finalize(stmt);
	dbVerify(ret);
}

std::string DBManager::getJobSearchResult(std::string query)
{
	mtx.lock();   ///////////////////


	if(query.find('|') == query.npos){
		return "";
		mtx.unlock();   //-----------------
	}
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

	mtx.unlock();   //-----------------

	if(out.length() > 0)
		return out.substr(1);
	return "";
}

FractalMeta DBManager::getFractal(int jid){
	mtx.lock();   ////////////////////////
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

	mtx.unlock();   //-----------------
	return fract;
}

void DBManager::insertFractal(FractalMeta f){
	mtx.lock();   ////////////////////////
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

	mtx.unlock();   //-----------------
}

void DBManager::updateFractal(FractalMeta f)
{
	mtx.lock();   ////////////////////////
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

	mtx.unlock();   //-----------------
}


void DBManager::fillMDUDRequest(int jid, char **sz, int &len)
{
	try{
		int iTmp = 0;

		FractalMeta fract = getFractal(jid); // triggers a temp lock
		if(fract.jobID < 0){
			*sz = nullptr;
			len = 0;
			return;
		}

		// layout: [jid:4][uid:4][titleLen:4][title][authorLen:4][author][status:4][imgDtaLen:4][imgDta]
		// size: 24+|title| + |author| + |imgData|

		// open image
		std::string imgPath = concat(DirectoryManager::getSingleton()->getRootDirectory()+"renders/", jid) + ".png";
		FILE *fp = fopen(imgPath.c_str(), "rb");
		if(fp == NULL){ // no image
			len = 24 + fract.name.length() + fract.author.length();
			*sz = new char[len];
			memset(*sz + len - 4, 0, 4); // copy last four bytes as zero length of imgDta
		}else{
			fseek(fp, 0L, SEEK_END);
			int dtaSz = ftell(fp);
			fseek(fp, 0L, SEEK_SET);
			len = 24 + fract.name.length() + fract.author.length() + dtaSz;
			*sz = new char[len];

			iTmp = htonl(dtaSz);
			memcpy(*sz + len - 4 - dtaSz, &iTmp, 4); // copy last four bytes as zero length of imgDta
			char *ptr = *sz + len - dtaSz;
			for(int i=0; i<dtaSz; i++){
				char byte;
				fread(&byte, 1, 1, fp);
				*ptr = byte;
				ptr ++;
			}
			fclose(fp);
		}

		// now copy rest of data
		char *ptr = *sz;

		iTmp =htonl(fract.jobID);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(fract.userID);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(fract.name.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, fract.name.c_str(), fract.name.length());
		ptr += fract.name.length();

		iTmp = htonl(fract.author.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, fract.author.c_str(), fract.author.length());
		ptr += fract.author.length();

		iTmp = htonl(fract.status);
		memcpy(ptr, &iTmp, 4);

	}catch(std::bad_alloc &e){
		std::cerr << "Out of memory\n";
		exit(0);
	}
}


void DBManager::fillRCTXRequest(int jid, char **sz, int &len)
{
	try{
		int iTmp = 0;

		FractalMeta fract = getFractal(jid); // triggers a temp lock
		if(fract.jobID < 0){
			*sz = NULL;
			len = 0;
			return;
		}

		// layout: [jid:4][paramLen:4][params][poutLen:4][pOut][logLen:4][log][histLen:4][histo]
		// size: 20+|params| + |pOut| + |log| + |histo|

		std::string basePath = concat(DirectoryManager::getSingleton()->getRootDirectory()+"renders/", jid);

		// pOut
		Paramaters p;
		p.loadFromFile(basePath + ".info");
		std::string paramsOut = p.asString();

		// log
		std::fstream fp;
		std::string log = "";
		fp.open(std::string(basePath + ".log").c_str(), std::ios::in);
		if(fp.is_open()){
			std::string line;
			while(getline(fp,line))
			{
				log += line + "\n";
			}
		}

		// params----
		p.loadFromFile(basePath + ".job");
		std::string params = p.asString();

		ParamaterSchema schemBase(DirectoryManager::getSingleton()->getRootDirectory()+"SCHEMA_BASE");
		ParamaterSchema schemMandl(DirectoryManager::getSingleton()->getRootDirectory()+"SCHEMA_MULTIBROT");

		// histo
		int dtaSkip = schemBase.getInt(p, "imgWidth") * schemBase.getInt(p, "imgHeight") * 4;
		int iters = schemMandl.getInt(p, "iters");
		unsigned int *histogram = NULL;
		FILE *fp2 = fopen(std::string(basePath + ".algo").c_str(), "rb");
		if(iters > 0 && fp2 != NULL){
			fseek(fp2, dtaSkip, SEEK_SET);
			histogram = new unsigned int[iters];
			for(int i=0; i<iters; i++){
				int tmp;
				fread(&tmp, sizeof(unsigned int), 1, fp2);
				histogram[i] = htonl(tmp);
			}
		}
		if(fp2 != NULL)
			fclose(fp2);

		len = 20 + paramsOut.length() + params.length() + log.length();
		if(histogram != NULL)
			len += iters * 4;

		*sz = new char[len];
		char *ptr = *sz;

		iTmp =htonl(jid);
		memcpy(ptr, &iTmp, 4);
		ptr += 4;

		iTmp =htonl(params.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, params.c_str(), params.length());
		ptr += params.length();

		iTmp =htonl(paramsOut.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, paramsOut.c_str(), paramsOut.length());
		ptr += paramsOut.length();

		iTmp =htonl(log.length());
		memcpy(ptr, &iTmp, 4);
		ptr += 4;
		memcpy(ptr, log.c_str(), log.length());
		ptr += log.length();

		if(histogram == NULL){
			iTmp = 0;
			memcpy(ptr, &iTmp, 4);
			ptr += 4;
		}else{
			iTmp = htonl(iters * 4);
			memcpy(ptr, &iTmp, 4);
			ptr += 4;

			memcpy(ptr, histogram, iters*4);
			ptr += iters * 4;
		}
	}catch(std::bad_alloc &e){
		std::cerr << "Out of memory\n";
		exit(0);
	}
}

unsigned int DBManager::submitJob(FractalMeta meta, Paramaters *p)
{
	int jid = 0;
	mtx.lock();
	lastID ++;

	meta.jobID = lastID;
	jid = meta.jobID;
	FractalContainer c;
	c.m = meta;
	c.p = p;

	Submitted.push_back(c);

	// need to unlock to enter insertFractal (lock not needed anymore anyway)
	mtx.unlock();
	insertFractal(c.m);

	return jid;
}

std::vector<FractalContainer> DBManager::fetchSubmittedJobs()
{
	std::vector<FractalContainer> cpy;
	mtx.lock();

	cpy = Submitted;
	Submitted.clear();

	mtx.unlock();
	return cpy;
}




