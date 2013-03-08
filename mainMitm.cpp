/*
 * mainMitm.cpp
 *
 *  Created on: May 27, 2012
 *      Author: matteo
 */

#include "mitmClass.h"

int main(int argc, char** argv)
{

	mitmClass mitm;

	//mitm.accessService( mitm.serverIpAddress, mitm.serverPortNumber );

	mitm.setupMaliciousService( mitm.mitmIpAddress, mitm.mitmPortNumber );

	mitm.maliciousServiceLoop();

	mitm.quitMaliciousService();

	cout << "Program mitm finished!!!!" << endl;

}



