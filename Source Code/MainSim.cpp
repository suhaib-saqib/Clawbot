#pragma once
#ifndef MAIN_SIM
#define MAIN_SIM
#include <iostream>
#include <iomanip>
#include <string>
#include "MySocket.h"
#include "Pkt_Def.h"

bool ExeComplete = false;

void CommandFunc(string ip, int port) {

	//My socket TCP and CLIENT configured
	MySocket CommandSocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, DEFAULT_SIZE);

	//Connect (3 way handshake)
	CommandSocket.ConnectTCP();

	//Generate a Packet of type PktDef based on the user input
	int cmd, dur = 0;
	while (!ExeComplete) {
		//menu
		cout << "COMMAND PACKET MENU\n";
		cout << "===================\n";
		cout << "1 = FORWARD\n" <<
			"2 = BACKWARD\n" <<
			"3 = RIGHT\n" <<
			"4 = LEFT\n" <<
			"5 = UP\n" <<
			"6 = DOWN\n" <<
			"7 = OPEN\n" <<
			"8 = CLOSE\n" <<
			"9 = SLEEP\n" <<
			"===================\n\n";

		//User enter input
		cout << "Enter Command: ";
		cin >> cmd;
		if (cmd != 9) {
			cout << "Enter Duration: ";
			cin >> dur;
		}

		//Pkt def and set
		PktDef pkt;
		MotorBody mb;

		int count = 0;
		switch (cmd) {
		case 1: pkt.SetCmd(CmdType::DRIVE);
			mb.Direction = FORWARD;
			break;
		case 2: pkt.SetCmd(CmdType::DRIVE);
			mb.Direction = BACKWARD;
			break;
		case 3: pkt.SetCmd(CmdType::DRIVE);
			mb.Direction = RIGHT;
			break;
		case 4: pkt.SetCmd(CmdType::DRIVE);
			mb.Direction = LEFT;
			break;
		case 5: pkt.SetCmd(CmdType::ARM);
			mb.Direction = UP;
			break;
		case 6: pkt.SetCmd(CmdType::ARM);
			mb.Direction = DOWN;
			break;
		case 7: pkt.SetCmd(CmdType::CLAW);
			mb.Direction = OPEN;
			break;
		case 8: pkt.SetCmd(CmdType::CLAW);
			mb.Direction = CLOSE;
			break;
		case 9: pkt.SetCmd(CmdType::SLEEP);
			break;
		}

		mb.Duration = dur;
		//Increment the PktCount number
		pkt.SetPktCount(count++);
		//Set pkt body
		pkt.SetBodyData((char*)&mb, 2);
		//Generate CRC
		pkt.CalcCRC();

		//Buffer for recv
		char* rxBuffer = new char[DEFAULT_SIZE];
		//Transmit Packet
		CommandSocket.SendData(pkt.GenPacket(), pkt.GetLength());
		//put data in rxbuffer
		CommandSocket.GetData(rxBuffer);
		//ptr to command byte
		char * ptr = rxBuffer;
		ptr += sizeof(int);

		//if NACK
		if (*ptr == 0) {
			CommandSocket.SendData(pkt.GenPacket(), pkt.GetLength());
			CommandSocket.GetData(rxBuffer);
		}

		PktDef rec(rxBuffer);

		if (rec.GetCmd() == SLEEP) {
			cout << "\n====Command Thread has Disconnected====\n";
			CommandSocket.DisconnectTCP();
			ExeComplete = true;
		}
	}//while loop end
}

void telemetry(string ip, int port) {
	//starting and connecting socket
	MySocket telemetrySocket(SocketType::CLIENT, ip, port, ConnectionType::TCP, DEFAULT_SIZE);
	telemetrySocket.ConnectTCP();

	//loop until other thread completes and sets complete to true
	while (!ExeComplete) {
		//buffer for recieving
		char buff[128];
		int size = telemetrySocket.GetData(buff);

		//construct with recieved buffer
		PktDef telemetryPacket(buff);

		//validate crc
		if (!telemetryPacket.CheckCRC(buff, size)) {
			cout << "CRC mismatch!" << endl;
		}
		else {
			//crc is ok
			cout << "CRC good!" << endl;

			//check status
			if (telemetryPacket.GetCmd() == STATUS) {
				//status good
				//Display raw data packet
				cout << "Raw Data: " << endl;
				char *p = buff;
				for (int i = 0;i < size;i++) {
					cout << hex << (int)*(p++) << endl;
				}

				//display data
				char* ptr = telemetryPacket.GetBodyData();

				cout << dec << "Sonar: " << (int)*ptr << endl;
				ptr += 2;

				cout << "Arm: " << *(short*)ptr << endl;
				ptr += 2;

				char ptr2 = ptr[0]; //dereference ptr and assign to char array ptr2
				cout << "Drive Flag: " << (ptr2 & 1) << endl;

				if ((ptr2 >> 1) & 1) {
					cout << "Arm is up, ";
				}
				else if ((ptr2 >> 2) & 1) {
					cout << "Arm is down, ";
				}

				if ((ptr2 >> 3) & 1) {
					cout << " Claw is open." << endl;
				}
				else if ((ptr2 >> 4) & 1) {
					cout << " Claw is closed." << endl;
				}
			}
			else {
				cout << "STATUS not set to true!" << endl;
			}
		}
	}
}

int main() {
	//Roana
	string ip;
	int cport;

	//Query user for required information
	cout << "Enter IP: ";
	cin >> ip;

	cout << "Enter Command Port: ";
	cin >> cport;

	// getting input for port and IP
	int tport;
	cout << "Enter Telemetry Port: ";
	cin >> tport;

	//first thread
	thread t1(CommandFunc, ip, cport);
	t1.detach();

	//second thread
	thread t2;
	t2 = thread(telemetry, ip, tport);
	t2.detach();

	while (1) {
		
	}
	system("pause");
	return 1;
}
#endif