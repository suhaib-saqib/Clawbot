#pragma once
#pragma once
#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#ifndef MY_SOCKET
#define MY_SOCKET
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

enum SocketType { CLIENT, SERVER };

enum ConnectionType { TCP, UDP };

#define DEFAULT_SIZE 100

class MySocket {
	char* Buffer;
	SOCKET WelcomeSocket;
	SOCKET ConnectionSocket;
	sockaddr_in SvrAddr;
	SocketType mySocket;
	string IPAddr;
	int Port;
	ConnectionType connectionType;
	bool bTCPConnect;
	int MaxSize;

public:
	MySocket(SocketType, string, unsigned int, ConnectionType, unsigned int);
	~MySocket();
	void ConnectTCP();
	void DisconnectTCP();
	void SendData(const char*, int);
	int GetData(char*);
	string GetIPAddr();
	void SetIPAddr(string);
	void SetPort(int);
	int GetPort();
	SocketType GetType();
	void SetType(SocketType);
};
#endif