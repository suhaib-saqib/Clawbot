#include "MySocket.h"

MySocket::MySocket(SocketType _socket, string ip, unsigned int port, ConnectionType connection, unsigned int size) {
	//sets MySocket variables
	mySocket = _socket;
	IPAddr = ip;
	Port = port;
	connectionType = connection;

	//if invalid size entered, set to default size
	if (size > 128) {
		MaxSize = DEFAULT_SIZE;
	}
	else {
		MaxSize = size;
	}

	//allocates memory to buffer of MaxSize
	Buffer = new char[MaxSize];
	memset(Buffer, '-', MaxSize);
	//startup
	WSADATA wsaData;
	if ((WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		std::cout << "Could not start DLLs" << std::endl;
		std::cin.get();
		exit(0);
	}
	if (connectionType == UDP){
		WelcomeSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}
	else{
		WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	if (WelcomeSocket == INVALID_SOCKET)
	{
		WSACleanup();
		std::cout << "Could not initialize socket" << std::endl;
		std::cin.get();
		exit (0);
	}
	//bind
	SvrAddr.sin_family = AF_INET; //Address family type internet
	SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
	SvrAddr.sin_addr.s_addr = inet_addr(this->IPAddr.c_str()); //IP address
	if (mySocket == SERVER){
		if (::bind(WelcomeSocket, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR){
			closesocket(this->WelcomeSocket);
			WSACleanup();
			std::cout << "Could not bind to the socket" << std::endl;
			std::cin.get();
			exit(0);
		}
		if (connectionType == TCP){
			//listen
			if (listen(this->WelcomeSocket, 1) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not listen to the provided socket." << std::endl;
				std::cin.get();
				exit(0);
			}
			else {
				std::cout << "Waiting for client connection" << std::endl;
			}
			//accept
			if ((this->ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
				closesocket(this->WelcomeSocket);
				WSACleanup();
				std::cout << "Could not accept incoming connection." << std::endl;
				std::cin.get();
				exit(0);
			}
			else {
				std::cout << "Connection Accepted" << std::endl;
			}
		}
		else {
			ConnectionSocket = WelcomeSocket;
		}
	}
	else{
		ConnectionSocket = WelcomeSocket;
	}
}

//server socket destructor that closes server socket
MySocket::~MySocket() {
	this->DisconnectTCP();
}

void MySocket::ConnectTCP() {
	if (mySocket == TCP){
		if (connect(ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr)) == SOCKET_ERROR){
			closesocket(this->ConnectionSocket);
			WSACleanup();
			std::cout << "Could not connect to the server" << std::endl;
			std::cin.get();
			exit(0);
		}
	}
}

void MySocket::DisconnectTCP() {
	WSACleanup();
	closesocket(this->ConnectionSocket);
}

void MySocket::SendData(const char* data, int size) {
	//send function
	//if sockettype is TCP
	if (GetType() == 0) {
		send(ConnectionSocket, data, size, 0);
	}
	//else if sockettype is UDP
	else if (GetType() == 1) {
		sendto(ConnectionSocket, data, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr));
	}
}

int MySocket::GetData(char *data) {
	//recv function
	int bytes;
	//if sockettype is TCP
	if (GetType() == 0) {
		bytes = recv(ConnectionSocket, Buffer, MaxSize, 0);
	}
	//else if sockettype is UDP
	else if (GetType() == 1) {
		int size = sizeof(SvrAddr);
		bytes = recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&SvrAddr, &size);
	}
	memcpy(data, Buffer, bytes);
	return bytes;
}

string MySocket::GetIPAddr() {
	return IPAddr;
}

void MySocket::SetIPAddr(string ip) {
	if (bTCPConnect) {
		cout << "ERROR! Connection already established! Cannot change IP" << endl;
	}
	else {
		IPAddr = ip;
	}
}

void MySocket::SetPort(int prt) {
	if (bTCPConnect) {
		cout << "ERROR! Connection already established! Cannot change Port" << endl;
	}
	else {
		Port = prt;
	}
}

int MySocket::GetPort() {
	return Port;
}

SocketType MySocket::GetType() {
	return mySocket;
}

void MySocket::SetType(SocketType skt) {
	mySocket = skt;
}