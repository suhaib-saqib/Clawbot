#pragma once
#include <windows.networking.sockets.h>
#pragma comment(lib, "Ws2_32.lib")
#ifndef PKT_DEF
#define PKT_DEF
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

#define FORWARD 1
#define BACKWARD 2
#define RIGHT 3
#define LEFT 4
#define UP 5
#define DOWN 6
#define OPEN 7
#define CLOSE 8
#define HEADERSIZE 6

enum CmdType { DRIVE, STATUS, SLEEP, ARM, CLAW, ACK, NACK };

struct MotorBody {
	char Direction;
	char Duration;
};

struct Header {
	unsigned int PktCount;
	unsigned char Drive : 1;
	unsigned char Status : 1;
	unsigned char Sleep : 1;
	unsigned char Arm : 1;
	unsigned char Claw : 1;
	unsigned char Ack : 1;
	unsigned char Pad : 2;
	unsigned char Length;
};

struct CmdPkt {
	Header header;
	char * Data;
	unsigned char CRC;
};

class PktDef {
	CmdPkt packet;
	char * RawBuffer;

public:
	PktDef();
	PktDef(char*);
	void SetCmd(CmdType);
	void SetBodyData(char*, int);
	void SetPktCount(int);
	CmdType GetCmd();
	bool GetAck();
	int GetLength();
	char* GetBodyData();
	int GetPktCount();
	bool CheckCRC(char*, int);
	void CalcCRC();
	char* GenPacket();
}; 
#endif