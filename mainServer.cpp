/*
 * mainServer.cpp
 *
 *  Created on: May 26, 2012
 *      Author: matteo
 */

#include "serverClass.h"

int main(int argc, char** argv)
{

	serverClass server;

	server.setupService( server.serverIpAddress, server.serverPortNumber );

	server.serviceLoop();

	server.quitService();

	cout << "Program Server finished!!!!" << endl;
}


