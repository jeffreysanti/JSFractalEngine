/*
 * FTPServer.cpp
 *
 *  Created on: Jun 8, 2015
 *      Author: jeffrey
 */

#include "FTPServer.h"

#ifdef WIN32
	#define __USE_FILE_OFFSET64
#endif

#include "DirectoryManager.h"
#include <thread>

FTPServer FTPServer::singleton;

FTPServer::FTPServer() {
	portno = 0;
	nosockets = false;
}

FTPServer::~FTPServer() {
}

void FTPServer::initialize() {
	Json::Value &root = DirectoryManager::getSingleton()->getConfigRoot();
	if(!root.isMember("ftpport") || !root["ftpport"].isInt()){
		std::cerr << "Configuration 'ftpport' invalid or non-existent!\n";
		exit(EXIT_FAILURE);
		return;
	}
	portno = root["ftpport"].asInt();

	if(root.isMember("nosockets") && root["nosockets"].isBool() && root["nosockets"].asBool()){
		nosockets = true;
		return;
	}

	CFtpServer *FtpServer = new CFtpServer();
	FtpServer->SetMaxPasswordTries(3);
	FtpServer->SetNoLoginTimeout(45);
	FtpServer->SetNoTransferTimeout(90);
	FtpServer->SetDataPortRange(10000, 50000);
	FtpServer->SetCheckPassDelay(500);

	CFtpServer::CUserEntry *pAnonymousUser = FtpServer->AddUser("anonymous", NULL,
			std::string(DirectoryManager::getSingleton()->getRootDirectory()+"renders/").c_str());
	if(!pAnonymousUser){
		std::cerr << "pAnonymousUser failed!\n";
		exit(EXIT_FAILURE);
		return;
	}

	pAnonymousUser->SetPrivileges(CFtpServer::LIST | CFtpServer::READFILE);

	std::thread run = std::thread(&FTPServer::runServer, this, FtpServer);
	run.detach();
}

void FTPServer::runServer(CFtpServer *FtpServer){
	if( FtpServer->StartListening(INADDR_ANY, portno) ) {
		if( FtpServer->StartAccepting() ) {
			printf( "FTP SERVER Started Accepting\n" );
			for( ;; )
				#ifdef WIN32
					Sleep( 1000 );
				#else
					sleep( 1 );
				#endif
		}else{
			std::cerr << "FTP Unable to accept connections!\n";
			exit(EXIT_FAILURE);
			return;
		}
		FtpServer->StopListening();
	}else{
		std::cerr << "FTP Unable to listen!\n";
		exit(EXIT_FAILURE);
		return;
	}
	delete FtpServer;
}

int FTPServer::getPortNo(){
	if(nosockets)
		return -1;
	return portno;
}

FTPServer *FTPServer::getSingleton(){
	return &singleton;
}
