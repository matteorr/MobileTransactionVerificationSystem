/*
 * serverClass.cpp
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#include "serverClass.h"

#define MAX_BUF_SIZE 256

using namespace std;

serverClass::serverClass()
{

	QLEN = 1;
	endServiceLoop = 0;

	serverIpAddress = "127.0.0.1";
	serverPortNumber = 60000;

	privateKey = "0123456789:;<=>?@ABC";

}

serverClass::~serverClass()
{


}

void serverClass::setupService( string serverIpAddress, int serverPortNumber )
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
	checkOperation = inet_aton( serverIpAddress.c_str(), &sad.sin_addr );
	sad.sin_port = htons( (u_short)serverPortNumber );

	cout << "Server IP address and service port: [" << serverIpAddress << "]:[" << serverPortNumber << "]" << endl;
	cout << "Server IP address and service port: [" << sad.sin_addr.s_addr << "]:[" << sad.sin_port << "]" << endl;

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

void serverClass::serviceLoop()
{

	/*
	 * variable to check operations
	 */
	int checkOperation;

	/*
	 * variable for transaction freshness
	 */
	time_t nonce;

	/*
	 * buffer to receive data from clients
	 */
	char buffer[MAX_BUF_SIZE];

	/*
	 * string representing incoming transactions
	 */
	string transactionData;

	/*
	 * string representing the acknowledgment message of transaction authentication
	 */
	string clientAck;

	cout << "Server in the service loop" << endl;

	while( !endServiceLoop ){

		alen = sizeof(cad);

		cout << "Server is waiting for a Client to serve..." << endl << endl;

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

		cout << "Serving Client with the following Socket Descriptor [" << clientSocketDescriptor << "]" << endl;
		cout << "Waiting for a transaction from the Client..." << endl << endl;

		/*
		 * emptying buffer for new incoming data
		 */
		for(int i = 0; i < MAX_BUF_SIZE; i++){
			buffer[i] = 0;
		}

		/*
		 * blocking function waiting for incoming data from client
		 */
		checkOperation = read( clientSocketDescriptor, buffer, sizeof(buffer) );

		nonce = time(0);

		cout << "Transaction received from client: [" << buffer << "] with timestamp [" << nonce << "]" << endl;
		transactionData = string( buffer );

		/*
		 * calling authentication method
		 */
		checkOperation = this->transactionAuthentication( transactionData, nonce );

		if( checkOperation == 1 ){
			//transaction was accepted by the OOB client
			clientAck = "Your transaction was successfully completed!";
		}else{
			//transaction was rejected by the OOB client
			clientAck = "Your Transaction was maliciously altered by an hacker and rejected!Please try again!";
		}
		write( clientSocketDescriptor, clientAck.c_str(), clientAck.length() );

		/*
		 * closing connection with client once served
		 */
		close( clientSocketDescriptor );
	}

	cout << "Out of the service loop" << endl;

}

void serverClass::quitService()
{

	cout << endl << "Shutting Down e-Banking Service!" << endl;
	close( socketDescriptor );

}

int serverClass::transactionAuthentication( string transactionData, time_t nonce )
{

	/*
	 * variable to check operations
	 */
	int checkOperation;

	/*
	 * file descriptors for the named Pipes
	 */
	int readPipeDescriptor, writePipeDescriptor;

	/*
	 * buffers to receive data from clients
	 */
	char buffer[MAX_BUF_SIZE];
	string receivedHash, receivedIban, receivedAmount, receivedNonce;

	/*
	 * integer amount
	 */
	int amountValue;

	/*
	 * strings containing the accept and reject hash
	 * must be compared to the hash received by OOB client
	 */
	string acceptHash, rejectHash;

	transactionData = transactionData + "NONCE[" + to_string( nonce ) + "]";

	cout << "Authenticating and signing received transaction: [" << transactionData << "]" << endl << endl;

	/*
	 * Create the named pipes
	 * which simulate MA-SSL communication channel with OOB client
	 */
	checkOperation = mkfifo( "/tmp/pipeServer2OOBClient", 0666 );
	if(( checkOperation == -1) /*&& (errno != EEXIST)*/ ){
		perror("Error creating the named pipe");
	    exit(1);
	}

	checkOperation = mkfifo( "/tmp/pipeOOBClient2Server", 0666 );
	if(( checkOperation == -1) /*&& (errno != EEXIST)*/ ){
		perror("Error creating the named pipe");
		exit(1);
	}

	/*
	 * Open the first named pipe for writing
	 */
	writePipeDescriptor = open( "/tmp/pipeServer2OOBClient", O_WRONLY );

	/*
	 * Open the second named pipe for reading
	 */
	readPipeDescriptor = open( "/tmp/pipeOOBClient2Server", O_RDONLY);

	/*
	 * send transaction data to the OOB client
	 */
	write( writePipeDescriptor, transactionData.c_str(), transactionData.length() );

	/*
	 * receive authentication HASH from the OOB client
	 */
	checkOperation = read( readPipeDescriptor, buffer, MAX_BUF_SIZE );
	buffer[checkOperation] = '\0';
	receivedHash = string( buffer );

	cout << endl << "Received HASH from the OOB client: [" << receivedHash << "]" << endl << endl;

	/*
	 * closes Pipe and deletes file associated
	 * simulates the closing of the MA-SSL channel with OOB client
	 */
	close( writePipeDescriptor );
	close( readPipeDescriptor );

	if( remove( "/tmp/pipeServer2OOBClient" ) != 0 )
		perror( "Error deleting file" );
	else
		//cout << "File successfully deleted" << endl;

	if( remove( "/tmp/pipeOOBClient2Server" ) != 0 )
			perror( "Error deleting file" );
		else
			//cout << "File successfully deleted" << endl << endl;

	/*
	 * extract single parts of transaction
	 */
	receivedIban = this->extractData( transactionData, "IBAN" );
	receivedAmount = this->extractData( transactionData, "AMOUNT" );

	/*
	 * convert string to integer values
	 */
	amountValue = atoi( receivedAmount.c_str() );

	/*
	 * locally computing hash to understand if transaction was altered
	 */
	acceptHash = this->hashFunction( receivedIban, amountValue, nonce, +1 );
	rejectHash = this->hashFunction( receivedIban, amountValue, nonce, -1 );

	if( receivedHash.compare( acceptHash ) == 0 ){
		checkOperation = 1;
		cout << "Transaction was accepted by End User on the OOB channel" << endl << endl;
	}else if( receivedHash.compare( rejectHash ) == 0 ){
		checkOperation = -1;
		cout << "Transaction was rejected by End User on the OOB channel" << endl << endl;
	}else{
		cout << "ERROR in Hash Calculation!" << endl;
		cout << "Received Hash: [" << receivedHash << "]" << endl;
		cout << "Accept Hash: [" << acceptHash << "]" << endl;
		cout << "Reject Hash: [" << rejectHash << "]" << endl << endl;
		exit(1);
	}

	return checkOperation;

}

string serverClass::extractData( string transactionData, string wantedData)
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

string serverClass::hashFunction( string receivedIban, int receivedAmount, time_t nonce, int confirmationChoice )
{

	//cout << "transaction data to HASH is: " << receivedAmount << " - " << receivedIban << " - " << confirmationChoice << " - " << nonce << endl;

	string hexDigest;

	string inputString;

	inputString += receivedIban + to_string( receivedAmount ) + to_string( nonce ) + to_string( confirmationChoice );
	//cout << "transaction data to HASH is: " << inputString << endl;

	hexDigest = this->HMAC( inputString );

	std::stringstream ss;

	/*
	 * converting bit string to hexadecimal values
	 */
	int value, k = 0;
	std::bitset<160> finalDigest( hexDigest );

	for(int i = 39; i >= 0; i--){
		value = 0;
		for(int j = 3; j >= 0; j--){
			//cout << finalDigest[i * 4 + j] << " * " << pow(2, j) << endl;
			value += finalDigest[i * 4 + j] * pow(2, j);
			k++;
		}

		if( k == 32 ){
			ss << std::hex << value << " ";
			//printf(" ");
			k = 0;
		}else{
			ss << std::hex << value;
			//printf("%x", value);
		}

	}

	hexDigest = string( ss.str() );
	//cout << "Final Digest in HEX values: " << hexDigest << endl;

	return hexDigest;

}

string serverClass::HMAC( string inputString )
{
	//cout << "in HMAC" << endl;

	/*
	 * number of characters in input string
	 */
	int inputLength;

	/*
	 * bitset representing the padded key used in the HMAC
	 * constructed from the PrivateKey
	 */
	std::bitset<512> K0;

	/*
	 * opad be the outer padding (0x5c5c5c…5c5c, one-block(512 bit)-long hexadecimal constant)
	 * ipad be the inner padding (0x363636…3636, one-block(512 bit)-long hexadecimal constant)
	 */
	std::bitset<512> o_key_pad;
	std::bitset<512> i_key_pad;

	/*
	 * bitset vector for temporary operations
	 */
	std::bitset<512> tempBitVector;

	/*
	 * string for constructing the complete private key representation
	 */
	string privateKeyBinaryRepresentation;
	string oKeyPadBinaryRepresentation;
	string iKeyPadBinaryRepresentation;

	string tempBinaryRepresentation;

	string digest;

	/*
	 * length in number of characters of the private key
	 */
	int privateKeyLength = this->privateKey.length();
	//cout << "lunghezza byte della chiave: " << privateKeyLength;

	/*
	 * variable to access elements of the input string
	 */
	const char* testCharStar = this->privateKey.c_str();
	//cout << "chiave: [" << this->privateKey << "][" << testCharStar << "]" << endl;

	/*
	 * string representing the binary 8 bit representation for each char of the input string
	 */
	string inputStringBinaryRepresentation;

	/*
	 * variable needed for HMAC initialization
	 * number of bytes used for the padding of the key
	 */
	int numberOfPaddingBytes;

	if( privateKeyLength == 64){

		//cout << "chiave lunga 64 byte" << endl;

		for(int i = 0; i < privateKeyLength; i++){

			//cout << *testCharStar << endl;
			//printf("char:[%c] - asciiDEC:[%d] - asciiHEX:[%x] - ", *testCharStar, *testCharStar, *testCharStar);

			/*
			 * 8 bit variable corresponding to a char in the input string
			 */
			std::bitset<8> inputChar( *testCharStar );

			//cout << inputChar << endl;

			/*
			 * concatenation of all char representation to obtain bit string corresponding to input string
			 */
			privateKeyBinaryRepresentation += inputChar.to_string();
			testCharStar++;
		}

		K0 = std::bitset<512>( privateKeyBinaryRepresentation );
		//cout << endl << endl << privateKeyBinaryRepresentation << endl << K0 << endl << endl;
	}

	if( privateKeyLength < 64){

		//cout << "chiave lunga meno di 64 byte" << endl;

		for(int i = 0; i < privateKeyLength; i++){

			//cout << *testCharStar << endl;
			//printf("char:[%c] - asciiDEC:[%d] - asciiHEX:[%x] - ", *testCharStar, *testCharStar, *testCharStar);

			/*
			 * 8 bit variable corresponding to a char in the input string
			 */
			std::bitset<8> inputChar( *testCharStar );

			//cout << inputChar << endl;

			/*
			 * concatenation of all char representation to obtain bit string corresponding to input string
			 */
			privateKeyBinaryRepresentation += inputChar.to_string();
			testCharStar++;
		}

		//cout << "private key binary representation: " << privateKeyBinaryRepresentation << endl;

		numberOfPaddingBytes = 64 - privateKeyLength;

		/*
		 * procedure to get the full key
		 */
		for( int i = 0; i < numberOfPaddingBytes; i++ ){

			/*
			 * numberOfPaddingBytes zero bytes are added
			 */

			std::bitset<8> inputByte( string("00000000") );
			privateKeyBinaryRepresentation += inputByte.to_string();

		}

		K0 = std::bitset<512>( privateKeyBinaryRepresentation );

	}

	if( privateKeyLength > 64 ){
		cout << endl << "set a shorter private key cause having it this long doesn't add security but just complexity!!!" << endl;
		exit(1);
	}

	/*
	 * procedure to initialize iKeyPad and oKeyPad
	 */
	for( int i = 0; i < 64; i++ ){

		/*
		 * numberOfPaddingBytes zero bytes are added
		 */

		std::bitset<8> iKeyPadByte( string("00110110") );
		iKeyPadBinaryRepresentation += iKeyPadByte.to_string();

		std::bitset<8> oKeyPadByte( string("01011100") );
		oKeyPadBinaryRepresentation += oKeyPadByte.to_string();

	}

	i_key_pad = std::bitset<512>( iKeyPadBinaryRepresentation );
	o_key_pad = std::bitset<512>( oKeyPadBinaryRepresentation );

	tempBitVector = K0 ^ i_key_pad;

	inputLength = inputString.length();
	testCharStar = inputString.c_str();
	//cout << testCharStar << " - " << inputLength << endl;

	/*
	 * first part of prepared message is the bit representation of the input
	 */
	for(int i = 0; i < inputLength; i++){

		//cout << *testCharStar << endl;
		//printf("char:[%c] - asciiDEC:[%d] - asciiHEX:[%x] - ", *testCharStar, *testCharStar, *testCharStar);

		/*
		 * 8 bit variable corresponding to a char in the input string
		 */
		std::bitset<8> inputChar( *testCharStar );

		//cout << inputChar << endl;

		/*
		 * concatenation of all char representation to obtain bit string corresponding to input string
		 */
		inputStringBinaryRepresentation += inputChar.to_string();
		testCharStar++;
	}

	tempBinaryRepresentation = tempBitVector.to_string() + inputStringBinaryRepresentation;

	digest = this->SHA1( tempBinaryRepresentation, 0 );

	tempBitVector = K0 ^ o_key_pad;

	tempBinaryRepresentation = tempBitVector.to_string() + digest;

	digest = this->SHA1( tempBinaryRepresentation, 0 );

	return digest;

}


string serverClass::SHA1( string inputString, int mode )
{

	/*
	 * SHA-1 initialization variables
	 */
	unsigned long int h0Int = 1732584193; //0x67452301
	std::bitset<32> h0( h0Int );

	unsigned long int h1Int = 4023233417; //0xefcdab89
	std::bitset<32> h1( h1Int );

	unsigned long int h2Int = 2562383102; //0x98badcfe
	std::bitset<32> h2( h2Int );

	unsigned long int h3Int = 271733878; //0x10325476
	std::bitset<32> h3( h3Int );

	unsigned long int h4Int = 3285377520; //0xc3d2e1f0
	std::bitset<32> h4( h4Int );

	/*
	printf("%d-%x", h0Int, h0Int);
	cout << " - bitset - " << h0 << endl;

	printf("%d-%x", h1Int, h1Int);
	cout << " - bitset - " << h1 << endl;

	printf("%d-%x", h2Int, h2Int);
	cout << " - bitset - " << h2 << endl;

	printf("%d-%x", h3Int, h3Int);
	cout << " - bitset - " << h3 << endl;

	printf("%d-%x", h4Int, h4Int);
	cout << " - bitset - " << h4 << endl;
	*/

	/*
	 * number of 512 bits chunks contained in the complete message string once padded
	 */
	int c = -1;

	/*
	 * string representing the binary 8 bit representation for each char of the input string
	 */
	string inputStringBinaryRepresentation;

	c = this->SHA1_MessagePreparation( inputString, inputStringBinaryRepresentation, mode );

	//cout << endl << endl << endl << inputStringBinaryRepresentation << endl << endl << endl;

	/*
	 * vector needed to contain all the 32 bit words composing the message
	 */
	std::vector< std::bitset<32> > bitsetVector;

	/*
	 * 32 bit variables for the calculation of the hash value
	 */
	std::bitset<32> aBitVector;
	std::bitset<32> bBitVector;
	std::bitset<32> cBitVector;
	std::bitset<32> dBitVector;
	std::bitset<32> eBitVector;

	std::bitset<32> tempBitVector;
	std::bitset<32> fBitVector;
	std::bitset<32> kBitVector;

	/*
	 * variable needed for the modulo 2^32 sums (must be 64 bit)
	 */
	unsigned long long int tempInt;

	/*
	 * variable for containing the 512 bit chunks composing the full message binary string
	 */
	string inputStringBinaryChunk;

	/*
	 * procedure for the calculation of the SHA-1 value
	 * from FIPS-180 document
	 */
	for( int i = 0; i < c + 1; i++ ){

		//cout << endl << "---------------------------------------------------------------------------------------" << endl;
		//cout << i << endl << endl;

		bitsetVector.clear();

		inputStringBinaryChunk = inputStringBinaryRepresentation.substr( i * 512, 512 );

		//cout << inputStringBinaryChunk << endl;

		for( int j = 0; j < 80; j++ ){

			if( j < 16 ){
				//cout << j << endl;
				//cout << "string" << inputStringBinaryChunk.substr( j * 32, 32 ) << endl;
				/*
				 * the 512 bit string is divided in 16 words of 32 bit
				 */
				bitsetVector.push_back( std::bitset<32>( inputStringBinaryChunk.substr( j * 32, 32 ) ) );
				//cout << "bitset" << bitsetVector[j] << endl;
			}else{
				//cout << j << endl;
				//cout << (bitsetVector[j-3] ^ bitsetVector[j-8] ^ bitsetVector[j-14] ^ bitsetVector[j-16]) << endl;
				std::bitset<32> result;
				result = ((bitsetVector[j-3] ^ bitsetVector[j-8] ^ bitsetVector[j-14] ^ bitsetVector[j-16])<<1) | ((bitsetVector[j-3] ^ bitsetVector[j-8] ^ bitsetVector[j-14] ^ bitsetVector[j-16])>>31);
				//cout << result << endl;

				/*
				 * the 512 bit chunk is extended to 80 words of 32 bits
				 */
				bitsetVector.push_back( result );
			}
		}

		/*
		 * initialization of constant values for SHA-1
		 */
		aBitVector = h0;
		bBitVector = h1;
		cBitVector = h2;
		dBitVector = h3;
		eBitVector = h4;

		/*
		 * procedure on all of the 80 words to compute the message digest
		 */
		for( int j = 0; j < 80; j++ ){

			if( j >= 0 && j <= 19 ){

				fBitVector = (bBitVector & cBitVector) | ((~bBitVector) & dBitVector);
				kBitVector = std::bitset<32>( 1518500249ul ); //k = 0x5A827999
			}

			if( j >= 20 && j <= 39 ){

				fBitVector = (bBitVector ^ cBitVector ^ dBitVector);
				kBitVector = std::bitset<32>( 1859775393ul ); //k = 0x6ED9EBA1
			}

			if( j >= 40 && j <= 59 ){

				fBitVector = (bBitVector & cBitVector) | (bBitVector & dBitVector) | ( cBitVector & dBitVector );
				kBitVector = std::bitset<32>( 2400959708ul ); //k = 0x8F1BBCDC
			}

			if( j >= 60 && j <= 79 ){

				fBitVector = (bBitVector ^ cBitVector ^ dBitVector);
				kBitVector = std::bitset<32>( 3395469782ul ); //k = 0xCA62C1D6
			}

	//		cout << "addendo1: " << ( (aBitVector<<5) | (aBitVector>>27) ).to_ulong() << endl;
	//		cout << "addendo2: " << fBitVector.to_ulong() << endl;
	//		cout << "addendo3: " << eBitVector.to_ulong() << endl;
	//		cout << "addendo4: " << kBitVector.to_ulong() << endl;
	//		cout << "addendo5: " << bitsetVector[j].to_ulong() << endl;

			/*
			 * 64 bit sum
			 */
			tempInt = (long long int)( (aBitVector<<5) | (aBitVector>>27) ).to_ulong() + (long long int)fBitVector.to_ulong() + (long long int)eBitVector.to_ulong() + (long long int)kBitVector.to_ulong() + (long long int)bitsetVector[j].to_ulong();
	//		cout << "64 bit int - " << tempInt << endl;
	//		cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;

			/*
			 * result is made modulo 2^32
			 */
			tempBitVector = std::bitset<32>( tempInt % (long long int)4294967296 );
	//		cout << "32 bit vector -                                 " << tempBitVector << endl;

			/*
			 * variable update
			 */
			eBitVector = dBitVector;
			dBitVector = cBitVector;
			cBitVector = ( (bBitVector<<30) | (bBitVector>>2) );
			bBitVector = aBitVector;
			aBitVector = tempBitVector;

			/*if(j < 10)
				cout << "[" << j << "] -  " << aBitVector << " | " << bBitVector << " | " << cBitVector << " | " << dBitVector << " | " << eBitVector << endl;
			else
				cout << "[" << j << "] - " << aBitVector << " | " << bBitVector << " | " << cBitVector << " | " << dBitVector << " | " << eBitVector << endl;*/
		}

		/*
		 * accumulation of result on every 512 bit chunk
		 */

	//	cout << h0.to_ulong() << " + " << aBitVector.to_ulong() << " = " << h0.to_ulong() + aBitVector.to_ulong() << endl;
	//	printf(" %x + %x = %x\n", h0.to_ulong(), aBitVector.to_ulong(), h0.to_ulong() + aBitVector.to_ulong());

		tempInt = (long long int)h0.to_ulong() + (long long int)aBitVector.to_ulong();
	//	cout << "64 bit int - " << tempInt << endl;
	//	cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;
		h0 = std::bitset<32>( tempInt % (long long int)4294967296 );
	//	cout << "32 bit vector -                                 " << h0 << endl;

	//	cout << h1.to_ulong() << " + " << bBitVector.to_ulong() << " = " << h1.to_ulong() + bBitVector.to_ulong() << endl;
	//	printf(" %x + %x = %x\n", h1.to_ulong(), bBitVector.to_ulong(), h1.to_ulong() + bBitVector.to_ulong());

		tempInt = (long long int)h1.to_ulong() + (long long int)bBitVector.to_ulong();
	//	cout << "64 bit int - " << tempInt << endl;
	//	cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;
		h1 = std::bitset<32>( tempInt % (long long int)4294967296 );
	//	cout << "32 bit vector -                                 " << h1 << endl;

	//	cout << h2.to_ulong() << " + " << cBitVector.to_ulong() << " = " << h2.to_ulong() + cBitVector.to_ulong() << endl;
	//	printf(" %x + %x = %x\n", h2.to_ulong(), cBitVector.to_ulong(), h2.to_ulong() + cBitVector.to_ulong());

		tempInt = (long long int)h2.to_ulong() + (long long int)cBitVector.to_ulong();
	//	cout << "64 bit int - " << tempInt << endl;
	//	cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;
		h2 = std::bitset<32>( tempInt % (long long int)4294967296 );
	//	cout << "32 bit vector -                                 " << h2 << endl;

	//	cout << h3.to_ulong() << " + " << dBitVector.to_ulong() << " = " << h3.to_ulong() + dBitVector.to_ulong() << endl;
	//	printf(" %x + %x = %x\n", h3.to_ulong(), dBitVector.to_ulong(), h3.to_ulong() + dBitVector.to_ulong());

		tempInt = (long long int)h3.to_ulong() + (long long int)dBitVector.to_ulong();
	//	cout << "64 bit int - " << tempInt << endl;
	//	cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;
		h3 = std::bitset<32>( tempInt % (long long int)4294967296 );
	//	cout << "32 bit vector -                                 " << h3 << endl;

	//	cout << h4.to_ulong() << " + " << eBitVector.to_ulong() << " = " << h4.to_ulong() + eBitVector.to_ulong() << endl;
	//	printf(" %x + %x = %x\n", h4.to_ulong(), eBitVector.to_ulong(), h4.to_ulong() + eBitVector.to_ulong());

		tempInt = (long long int)h4.to_ulong() + (long long int)eBitVector.to_ulong();
	//	cout << "64 bit int - " << tempInt << endl;
	//	cout << "64 bit vector - " << std::bitset<64>(tempInt) << endl;
		h4 = std::bitset<32>( tempInt % (long long int)4294967296 );
	//	cout << "32 bit vector -                                 " << h4 << endl;

	}

	string binaryDigest;
	string hexDigest;
	std::stringstream ss;

	/*
	 * final concatenation of partial values
	 */
	binaryDigest = h0.to_string() + h1.to_string() + h2.to_string() + h3.to_string() + h4.to_string();

	//cout << "Final Digest: " << binaryDigest << endl;

	/*
	 * converting bit string to hexadecimal values
	 */
	int value, k = 0;
	std::bitset<160> finalDigest( binaryDigest );

	for(int i = 39; i >= 0; i--){
		value = 0;
		for(int j = 3; j >= 0; j--){
			//cout << finalDigest[i * 4 + j] << " * " << pow(2, j) << endl;
			value += finalDigest[i * 4 + j] * pow(2, j);
			k++;
		}

		if( k == 32 ){
			ss << std::hex << value << " ";
			//printf(" ");
			k = 0;
		}else{
			ss << std::hex << value;
			//printf("%x", value);
		}

	}

	hexDigest = string( ss.str() );
	//cout << "Final Digest in HEX values: " << hexDigest << endl;

	if( mode == 1 )
		return hexDigest;
	else
		return binaryDigest;

}


int serverClass::SHA1_MessagePreparation( string inputString, string& inputStringBinaryRepresentation, int mode )
{

	/*
	 * number of characters in input string
	 */
	int inputLength;

	/*
	 * 64 bitset representing the number of bits in the input string
	 */
	std::bitset<64> inputStringBinaryLength;

	/*
	 * variable to access elements of the input string
	 */
	const char* testCharStar;

	/*
	 * variables needed for SHA-1 initialization
	 * number of bytes and bits used for the padding of the message
	 */
	int numberOfPaddingBytes;
	int numberOfPaddingBits;

	/*
	 * number of 512 bits chunks contained in the complete message string once padded
	 */
	int c = -1;

	inputLength = inputString.length();

	if( mode == 1 ){

		inputStringBinaryLength = std::bitset<64>( inputLength * 8 );
		//cout << "[" << inputLength * 8 << "] =(on 64 bit) [" << inputStringBinaryLength << "]" << endl << endl;
		testCharStar = inputString.c_str();
		//cout << testCharStar << " - " << inputLength << endl;

		/*
		 * first part of prepared message is the bit representation of the input
		 */
		for(int i = 0; i < inputLength; i++){

			//cout << *testCharStar << endl;
			//printf("char:[%c] - asciiDEC:[%d] - asciiHEX:[%x] - ", *testCharStar, *testCharStar, *testCharStar);

			/*
			 * 8 bit variable corresponding to a char in the input string
			 */
			std::bitset<8> inputChar( *testCharStar );

			//cout << inputChar << endl;

			/*
			 * concatenation of all char representation to obtain bit string corresponding to input string
			 */
			inputStringBinaryRepresentation += inputChar.to_string();
			testCharStar++;
		}

		//cout << endl << "Complete Message Binary String - " << inputStringBinaryRepresentation << endl;

		do{

			c++;
			numberOfPaddingBits = (c * 512) + 448 -1 - (inputLength * 8);
		}while( numberOfPaddingBits < 0 );

		numberOfPaddingBytes = (numberOfPaddingBits + 1) / 8;

		//cout << "quante volte ci sta 512 in length of message: " << c << endl;
		//cout << "Padding bits: [1 + " << numberOfPaddingBits << "] PaddingBytes: [" << numberOfPaddingBytes << "]" << endl;

		/*
		 * procedure to get the full message
		 */
		for( int i = 0; i < numberOfPaddingBytes; i++ ){

			/*
			 * first added bit is 1 followed by numberOfPaddingBits - 1 zeroes
			 */
			if( i == 0 ){

				std::bitset<8> inputByte( string("10000000") );
				inputStringBinaryRepresentation += inputByte.to_string();
			}else{

				std::bitset<8> inputByte( string("00000000") );
				inputStringBinaryRepresentation += inputByte.to_string();
			}
		}

		/*
		 * last part of the message is composed by the length of the input string expressed on 64 bits
		 */
		inputStringBinaryRepresentation += inputStringBinaryLength.to_string();
		//cout << endl << "Complete Message Binary String - " << inputStringBinaryRepresentation << endl;
		//cout << inputStringBinaryRepresentation.length() << endl;
	}else{

		/*
		 * first part of prepared message is the input (its already in bit representation)
		 */
		inputStringBinaryRepresentation += inputString;

		inputStringBinaryLength = std::bitset<64>( inputLength );
		//cout << "[" << inputLength << "] =(on 64 bit) [" << inputStringBinaryLength << "]" << endl << endl;

		do{

			c++;
			numberOfPaddingBits = (c * 512) + 448 - 1 - inputLength;
		}while( numberOfPaddingBits < 0 );

		numberOfPaddingBytes = (numberOfPaddingBits + 1) / 8;

		//cout << "quante volte ci sta 512 in length of message: " << c << endl;
		//cout << "Padding bits: [1 + " << numberOfPaddingBits << "] PaddingBytes: [" << numberOfPaddingBytes << "]" << endl;

		/*
		 * procedure to get the full message
		 */
		for( int i = 0; i < numberOfPaddingBytes; i++ ){

			/*
			 * first added bit is 1 followed by numberOfPaddingBits - 1 zeroes
			 */
			if( i == 0 ){

				std::bitset<8> inputByte( string("10000000") );
				inputStringBinaryRepresentation += inputByte.to_string();
			}else{

				std::bitset<8> inputByte( string("00000000") );
				inputStringBinaryRepresentation += inputByte.to_string();
			}
		}

		/*
		 * last part of the message is composed by the length of the input string expressed on 64 bits
		 */
		inputStringBinaryRepresentation += inputStringBinaryLength.to_string();
		//cout << endl << "Complete Message Binary String - " << inputStringBinaryRepresentation << endl;
		//cout << inputStringBinaryRepresentation.length() << endl;
	}


	return c;

}
