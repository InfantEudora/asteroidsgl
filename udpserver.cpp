#include "udpserver.h"

tcpserver::tcpserver(){
	init();
};

tcpserver::~tcpserver(){
};

void tcpserver::init(void){
	printf("Initing UDPServer\n");
#ifdef _WIN32
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		
	}else{
		printf("Winsock Version: %u.%u\n",LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion) );
	}
#endif
	//Variables.
	server_port = 0;
}


udpserver::udpserver(){
	init();
};

udpserver::~udpserver(){
};

void udpserver::init(void){
	printf("Initing UDPServer\n");
#ifdef _WIN32
	WSADATA wsaData;
	int iResult;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		
	}else{
		printf("Winsock Version: %u.%u\n",LOBYTE(wsaData.wVersion),HIBYTE(wsaData.wVersion) );
	}
#endif
	//Variables.
	server_port = 0;
}


int udpserver::listen(int port){
	server_port = port;
	
	m_clientsocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(m_clientsocket == -1){
        printf("Error in creating socket\n");
        return 0;
    }
	    
    memset(&UDPserveraddr,0, sizeof(UDPserveraddr));
    UDPserveraddr.sin_family = AF_INET;
    UDPserveraddr.sin_port = htons(server_port);
    UDPserveraddr.sin_addr.s_addr = INADDR_ANY;
    
    int len = sizeof(UDPserveraddr);

    if(bind(m_clientsocket, (SOCKADDR*)&UDPserveraddr,sizeof(SOCKADDR_IN)) < 0){    
        printf("ERROR binding in the server socket\n");
		perror("socket error");
        return 0;
    }
	printf("UDP Socket is listening on port %i\n",port);
	return 1;
}


int udpserver::receive(int size, long ltimeout){
	fd_set fds;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = ltimeout;
	int data_len;
	

	//Loop
	while(1){
		FD_ZERO(&fds);
		FD_SET(m_clientsocket, &fds);

		int rc = select(sizeof(fds)*8, &fds, NULL, NULL, &timeout);
		//printf("TOUT: %i\n",timeout.tv_usec);
		if(rc > 0){			
			SOCKADDR_IN clientaddr;
			int len = sizeof(clientaddr);
			if((data_len = recvfrom(m_clientsocket,data, size, 0, (sockaddr*)&clientaddr, &len)) > 0){			
				//printf("UDP Data received from . data_len = %i\n",data_len);
				//printf("Client: %u.%u.%u.%u:%u\n",clientaddr.sin_addr.S_un.S_un_b.s_b1,clientaddr.sin_addr.S_un.S_un_b.s_b2,clientaddr.sin_addr.S_un.S_un_b.s_b3,clientaddr.sin_addr.S_un.S_un_b.s_b4,clientaddr.sin_port);
				/*for (int i =0;i<data_len;i++){
					putchar(data[i]);
				}*/

				char *p = inet_ntoa(clientaddr.sin_addr);
				int serverportno = ntohs(clientaddr.sin_port);
				return data_len;
			}	
		}else{
			//Nothing was selected. Break loop.
			return 0;
		}
	}
	return 0;
}

//udpclient

udpclient::udpclient(){
	init();
};

udpclient::~udpclient(){
};

void udpclient::init(void){


}

int udpclient::open(int port){
    server_port = port;
    


    client_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(client_socket == -1){
        printf("Error in creating socket\n");
        return 0;
    }

    char opt = 1; 
    setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(char));
    
    memset(&brdcastaddr,0, sizeof(brdcastaddr));
    brdcastaddr.sin_family = AF_INET;
    brdcastaddr.sin_port = htons(server_port);
    brdcastaddr.sin_addr.s_addr = INADDR_BROADCAST;
    
	return 1;
}

int udpclient::send(char* data, int len){
    int ret = sendto(client_socket, data, len, 0, (sockaddr*)&brdcastaddr, sizeof(brdcastaddr));
    if(ret < 0) {
         printf("Error broadcasting\n");
		 return 0;
    }else if(ret < len){
        printf("Not all data was sent\n");
		return len-ret;
    }else{/*
		printf("Following Data broadcasted: ");
		for (int i = 0;i<len;i++){
			printf("%02X ",(char)data[i]);
		}
		printf("\n");
		*/
		return len;
    }
	return 0;
}