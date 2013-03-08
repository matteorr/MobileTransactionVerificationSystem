/*
 * oobClientClass.h
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#ifndef OOBCLIENTCLASS_H_
#define OOBCLIENTCLASS_H_

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

class oobClientClass {

public:

	/*
	 * constructor
	 */
	oobClientClass();

	/*
	 * destructor
	 */
	virtual ~oobClientClass();

	/*
	 * authentication procedure in the following steps:
	 * - receive transaction data and nonce from server on MA-SSL OOB channel
	 * - ask confirmation to End User
	 * - calculate Hash Function (sign the transaction approval or rejection)
	 * - send digest to Server for definitive operation on transaction
	 */
	void authenticateTransaction();

private:

	string privateKey;

	/*
	 * extracts partial information from the full transaction
	 */
	string extractData( string transactionData, string wantedData );

	/*
	 * calculates the hash on the base of the following inputs and private key
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


#endif /* OOBCLIENTCLASS_H_ */
