/*
 * mainClient.cpp
 *
 *  Created on: May 27, 2012
 *      Author: matteo
 */

#include "clientClass.h"

int main(int argc, char** argv)
{

	clientClass client;

	int random = 0;

	client.makeTransaction();

	if( random >= 0.5 )
		client.accessService( client.serverIpAddress, client.serverPortNumber );
	else
		client.accessService( client.mitmIpAddress, client.mitmPortNumber );

	client.sendTransactionData();

	client.waitAcknowledgment();

	client.closeService();

	cout << "Program Client finished!!!!" << endl;

}





