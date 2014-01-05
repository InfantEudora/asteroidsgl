/*****
	udpserver.h
	Infant, 2012-03-17


*/
#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include <stdio.h>
//Include winsock2 before windows.h, or eveything will break.
#include <winsock2.h> 
#include <io.h>
#pragma comment(lib,"ws2_32.lib") 

class tcpserver{
	public:
		tcpserver();
		~tcpserver();
		void init();
		int listen(int port);
		int receive(int size, long ltimeout);

		char data[64];
	private:
		int server_port;
		SOCKET m_clientsocket;
		SOCKADDR_IN TCPserveraddr;
};

class udpserver{
	public:
		udpserver();
		~udpserver();
		void init();
		int listen(int port);
		int receive(int size, long ltimeout);

		char data[1024];
	private:
		int server_port;
		SOCKET m_clientsocket;
		SOCKADDR_IN UDPserveraddr;
};

class udpclient{
	public:
		udpclient();
		~udpclient();		
		void init();
		int open(int port);
		int send(char* data, int len);

		char data[1024];
	private:
		int server_port;
		int local_port;
		SOCKET client_socket;
		SOCKADDR_IN brdcastaddr;
};



#endif