/*
 * serverClass.h
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#ifndef SERVERCLASS_H_
#define SERVERCLASS_H_

//standard c libraries
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

//for timestamp
#include <ctime>

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

//for OS primitives
#include <unistd.h>

//for named pipes (mkfifo)
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <fcntl.h>

//for bit operations
#include "bitset"

//for string stream
#include <sstream>

using namespace std;

class serverClass {

public:

	string serverIpAddress;
	int serverPortNumber;

	struct sockaddr_in sad;
	struct sockaddr_in cad;
	socklen_t alen;

	int socketDescriptor;
	int clientSocketDescriptor;

	int QLEN;
	int endServiceLoop;

	/*
	 * constructor
	 */
	serverClass();

	/*
	 * destructor
	 */
	virtual ~serverClass();

	/*
	 * initializes service for incoming clients
	 */
	void setupService( string serverIpAddress, int serverPortNumber );

	/*
	 * serves a client in the following steps:
	 * - receives a transaction
	 * - authenticates transaction with OOB client
	 * - send acknowlegment message to client
	 */
	void serviceLoop();

	/*
	 * closes the e-banking service
	 */
	void quitService();

private:

	string privateKey;

	/*
	 * completes the procedure of OOB transaction verification and signing
	 * - input is the transaction data received from client
	 * - returns 1 if transaction was authenticated
	 * - returns -1 if transaction was modified
	 */
	int transactionAuthentication( string transactionData, time_t nonce );

	/*
	 * extracts partial information from the full transaction
	 */
	string extractData( string transactionData, string wantedData );

	/*
	 * localy calculates the hash of the transaction
	 * in case of approval or rejection by the End User
	 * - input is the hash received by the OOB client regarding the sent transaction and the authentication choice
	 * - returns the hash corresponding to that transaction data and to the authentication choice
	 */
	string hashFunction( string transactionIban, int transactionAmount, time_t nonce, int confirmationCode );

	/*
	 * calculates the hash on the base of the following inputs and private key
	 */
	string HMAC( string inputString );

	/*
	 * SHA-1 Hash function
	 * if mode = 1 then input is a string of chars
	 * if mode = 0 then input must be interpreted as the message in bits
	 * returned string will be string or bits depending from mode in the same way
	 */
	string SHA1( string inputString, int mode );

	/*
	 * SHA-1 Hash function message preparation
	 * given as input character string and a string representing the bit padded sequence of the input string
	 * returns as output the value of c
	 */
	int SHA1_MessagePreparation( string inputString, string& inputStringBinaryRepresentation, int mode );

};

#endif /* SERVERCLASS_H_ */
