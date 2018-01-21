#include "Pkt_Def.h"

PktDef::PktDef() {
	packet.header.PktCount = 0;
	packet.header.Drive = 0;
	packet.header.Status = 0;
	packet.header.Sleep = 0;
	packet.header.Arm = 0;
	packet.header.Claw = 0;
	packet.header.Ack = 0;
	packet.header.Pad = 0;
	packet.header.Length = 7;
	packet.Data = nullptr;
	packet.CRC = 0;
	RawBuffer = nullptr;
}

PktDef::PktDef(char* buffer) {
	char* ptr = buffer;
	memcpy(reinterpret_cast<void*>(&packet.header.PktCount), buffer, sizeof(int));
	ptr += sizeof(int);
	packet.header.Drive = (*ptr) & 1;
	packet.header.Status = (*ptr >> 1) & 1;
	packet.header.Sleep = (*ptr >> 2) & 1;
	packet.header.Arm = (*ptr >> 3) & 1;
	packet.header.Claw = (*ptr >> 4) & 1;
	packet.header.Ack = (*ptr >> 5) & 1;
	packet.header.Pad = 0;
	ptr += 1;
	memcpy(&packet.header.Length, ptr, sizeof(packet.header.Length));
	ptr += sizeof(packet.header.Length);

	//body arm claw 2
	//sleep ack and nack 0
	//status var

	if (/*packet.GetCmd() == BODY || packet.GetCmd() == ARM*/ (packet.header.Drive & 1) == 1 || (packet.header.Arm & 1) == 1 || /*packet.GetCmd() == CLAW*/ (packet.header.Claw & 1) == 1) {
		packet.Data = new char[2];
		memcpy(packet.Data, ptr, 2);
		ptr += 1;
	}
	else if (/*packet.GetCmd() == SLEEP*/ (packet.header.Sleep & 1) == 1 || /*packet.GetCmd() == ACK*/ (packet.header.Ack & 1) == 1 /*|| packet.GetCmd() == NACK*/) {
		memcpy(packet.Data, ptr, 0);
	}
	else if (/*packet.GetCmd() == STATUS*/ (packet.header.Status & 1) == 1) {
		packet.Data = new char[packet.header.Length - (HEADERSIZE + sizeof(char))];
		memcpy(packet.Data, ptr, (packet.header.Length - (HEADERSIZE + sizeof(char))));
		ptr += 1;
	}
	//memcpy(packet.Data, ptr, (packet.header.Length - sizeof(int) - 3));
	packet.CRC = *ptr;
}

void PktDef::SetCmd(CmdType cmd) {
	if (cmd == DRIVE) {
		packet.header.Drive = 1;
	}
	else if (cmd == STATUS) {
		packet.header.Status = 1;
	}
	else if (cmd == SLEEP) {
		packet.header.Sleep = 1;
	}
	else if (cmd == ARM) {
		packet.header.Arm = 1;
	}
	else if (cmd == CLAW) {
		packet.header.Claw = 1;
	}
	else if (cmd == ACK) {
		packet.header.Ack = 1;
	}
}

void PktDef::SetBodyData(char* dataInc, int size) {
	//set size depending on cmd type
	packet.Data = new char[size];

	memcpy(packet.Data, dataInc, sizeof(packet.Data));
	packet.header.Length += size;
}

void PktDef::SetPktCount(int count) {
	packet.header.PktCount = count;
}

CmdType PktDef::GetCmd() {
	char *ptr = (char*)&packet.header.PktCount+sizeof(packet.header.PktCount);
	if ((*ptr & 1) == 1) {
		return DRIVE;
	}
	else if ((*ptr >> 1)&1 == 1) {
		return STATUS;
	}
	else if ((*ptr >> 2)&1 == 1) {
		return SLEEP;
	}
	else if ((*ptr >> 3)&1 == 1) {
		return ARM;
	}
	else if ((*ptr >> 4)&1 == 1) {
		return CLAW;
	}
}

bool PktDef::GetAck() {
	char *ptr = (char*)&packet.header.PktCount + sizeof(packet.header.PktCount);
	if ((*ptr >> 5)&1 == 1) {
		return true;
	}
	else {
		return false;
	}
}

int PktDef::GetLength() {
	return (int)packet.header.Length;
}

char* PktDef::GetBodyData() {
	return packet.Data;
}

int PktDef::GetPktCount() {
	return packet.header.PktCount;
}

bool PktDef::CheckCRC(char* buffer, int size) {
	//counter to count the 1 bits
	int count = 0;
	//loop through each byte of the buffer
	for (int i = 0; i < size - 1; i++) {
		char bits = buffer[i];
		//loop through every bit in the byte
		for (int j = 0; j < 8; j++) {
			count += bits & 1;
			bits >>= 1;
		}
	}
	//copy CRC from buffer into an int
	int CRC = (int)buffer[size - 1];
	//compare CRC to calculated CRC
	if (CRC == count) {
		return true;
	}
	else {
		return false;
	}
}

void PktDef::CalcCRC() {
	//char* buffer;
	int count = 0;
	// loop though PktCount to count bits
	while (packet.header.PktCount) {
		count += packet.header.PktCount & 1;
		//shit bits to the right
		packet.header.PktCount >>= 1;
	}
	// add the flag bits ssince they can be accesed with variables
	count += packet.header.Drive + packet.header.Status + packet.header.Sleep + packet.header.Arm + packet.header.Claw + packet.header.Ack;
	//loop though Length byte
	unsigned char length = packet.header.Length;
	for (int i = 0; i < 8; i++) {
		count += length & 1;
		length >>= 1;
	}
	packet.CRC = count;
}

char* PktDef::GenPacket() {
	RawBuffer = new char[packet.header.Length];
	char* ptr = RawBuffer;
	memcpy(RawBuffer, reinterpret_cast<char*>(&packet.header), HEADERSIZE);
	ptr += HEADERSIZE;
	int size = (packet.header.Length - (HEADERSIZE + 1));
	if (size > 0) {
		memcpy(ptr, reinterpret_cast<char*>(packet.Data), size);
		ptr += size;
	}
	memcpy(ptr, reinterpret_cast<char*>(&packet.CRC), sizeof(char));
	return RawBuffer;
}