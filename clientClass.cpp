/*
 * clientClass.cpp
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#include "clientClass.h"

#define MAX_BUF_SIZE 256

using namespace std;

clientClass::clientClass(){

	mitmIpAddress = "127.0.0.1";
	mitmPortNumber = 60666;

	serverIpAddress = "127.0.0.1";
	serverPortNumber = 60000;

}

clientClass::~clientClass() {

}

void clientClass::makeTransaction()
{

	cout << "----------> Welcome to the Mobile Transaction Verification System Demo! <----------" << endl;
	cout << "Please insert the following details to simulate a e-Banking Transaction:" << endl;

	cout << endl << " - IBAN: ";
	cin >> this->transactionIban;

	cout << endl << " - AMOUNT ($): ";
	cin >> this->transactionAmount;

}

void clientClass::accessService( string ipAddress, int portNumber )
{

	/*
	 * variable to check operations
	 */
	int checkOperation;

	/*
	 * allocating memory for server socket address
	 */
	memset( (char*)&sad, 0, sizeof(sad) );

	/*
	 * setting socket parameters
	 */
	sad.sin_family = AF_INET;
	sad.sin_port = htons( (u_short)portNumber );
	checkOperation = inet_aton( ipAddress.c_str(), &sad.sin_addr );
	if( checkOperation == 0 ){
		cout << "Invalid IP format: [" << ipAddress << "]" << endl;
		exit(1);
	}

	/*
	 * creating socket to connect to the server
	 */
	cout << "Server to contact: [" << serverIpAddress << "]:[" << serverPortNumber << "]" << endl;
	cout << "Server to contact: [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

	socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);

	/*
	 * connecting to server through socket
	 */
	cout << "Connecting to [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

	checkOperation = connect( socketDescriptor, (struct sockaddr*)&sad, sizeof(sad) );
	if( checkOperation == 0 )
		cout << "Connected to [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl << endl;
	else{
		cout << "Unsuccessful connection, closing Client!" << endl;
		exit(1);
	}

}

void clientClass::sendTransactionData( /*string transactionIban, int transactionAmount*/ )
{

	/*
	 * string containing the transaction data previously created
	 */
	string transactionData;

	transactionData = "IBAN[" + this->transactionIban + "]AMOUNT[" + to_string( this->transactionAmount ) + "]";

	/*
	 * sending transaction on the created socket with server
	 */
	write( socketDescriptor, transactionData.c_str(), transactionData.length() );
	cout << "Sent Transaction to Server: [" << transactionData << "]" << endl << endl;

}

void clientClass::waitAcknowledgment()
{

	/*
	 * variable to check operations
	 */
	int checkOperation;

	/*
	 * buffer variable for receiving acknowledgment message from the server
	 */
	char buffer[MAX_BUF_SIZE];
	for(int i = 0; i < MAX_BUF_SIZE; i++){
		buffer[i] = 0;
	}

	checkOperation = read( socketDescriptor, buffer, sizeof(buffer) );
	cout << "Acknowledgment message from Server: [" << buffer << "]" << endl;

}

void clientClass::closeService()
{

	/*
	 * closing connection with server
	 */
	close( socketDescriptor );

}
