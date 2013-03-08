/*
 * clientClass.h
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#ifndef CLIENTCLASS_H_
#define CLIENTCLASS_H_

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

class clientClass {

public:

	string mitmIpAddress;
	string serverIpAddress;

	int mitmPortNumber;
	int serverPortNumber;

	struct sockaddr_in sad;
	socklen_t alen;

	int socketDescriptor;

	/*
	 * constructor
	 */
	clientClass();

	/*
	 * destructor
	 */
	virtual ~clientClass();

	/*
	 * procedure to get from input the transaction from client
	 */
	void makeTransaction();

	/*
	 * connects to the e-banking server (or to the MITM)
	 */
	void accessService( string ipAddress, int portNumber );

	/*
	 * sends the transaction data into the created socket over the network
	 */
	void sendTransactionData( /*string transactionIban, int transactionAmount*/ );

	/*
	 * client waits for acknowledgment of operation
	 */
	void waitAcknowledgment();

	/*
	 * closes the connection with the server
	 */
	void closeService();

private:

	string transactionIban;
	int transactionAmount;

};


#endif /* CLIENTCLASS_H_ */
