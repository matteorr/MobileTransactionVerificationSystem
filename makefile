all:serverMain clientMain mitmMain oobClientMain

serverMain:serverClass.o mainServer.o
	g++ -L/usr/local/lib -o serverMain mainServer.o serverClass.o

mainServer.o:mainServer.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c mainServer.cpp

serverClass.o:serverClass.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c serverClass.cpp

clientMain:clientClass.o mainClient.o
	g++ -L/usr/local/lib -o clientMain mainClient.o clientClass.o

mainClient.o:mainClient.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c mainClient.cpp

clientClass.o:clientClass.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c clientClass.cpp

mitmMain:mitmClass.o mainMitm.o
	g++ -L/usr/local/lib -o mitmMain mainMitm.o mitmClass.o

mainMitm.o:mainMitm.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c mainMitm.cpp

mitmClass.o:mitmClass.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c mitmClass.cpp

oobClientMain:oobClientClass.o mainOobClient.o
	g++ -L/usr/local/lib -o oobClientMain mainOobClient.o oobClientClass.o

mainOobClient.o:mainOobClient.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c mainOobClient.cpp

oobClientClass.o:oobClientClass.cpp
	g++ -I/usr/include/c++/4.5.2 -O3 -g3 -Wall -fmessage-length=0 -std=c++0x -c oobClientClass.cpp

clean:
	rm -rf *o serverMain clientMain mitmMain oobClientMain
