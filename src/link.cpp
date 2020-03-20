#include "link.h"

namespace FMS::Link {
	u8* buffer;
	u8 connectionId = 0;
	Handle recvFinishedEvent = 0;
	PrintConsole linkConsole;
	PrintConsole defaultConsole;
	Pcap* pcap;

	u8 secondShake[8] = { 0xA5, 0x00, 0x84, 0x01, 0x04, 0x04, 0x57, 0xd2 };
	u8 firstShake[8] = { 0xA5, 0x00, 0x84, 0x01, 0x03, 0x04, 0x70, 0x31 };

	//copied from keyboard example:
	static SwkbdState swkbd;
	static char mybuf[60];
	static SwkbdStatusData swkbdStatus;
	static SwkbdLearningData swkbdLearning;
	SwkbdButton button = SWKBD_BUTTON_NONE;
	bool didit = false;

	void startTransfer(int first = 0, bool editdata = false) {
		std::cout << ":: Start listening" << std::endl;

		int firstshakedone = 0;

		u8 dataSend[8];
		std::copy(firstShake, firstShake + 8, dataSend);
		u8 data[BUFFER_SIZE];

		if (editdata == true) {
			cout << ":: Note: this is a beta feature and will crash. Press (X) to continue and (Y) to go back";
			while (1) {
				if (hidKeysDown() & KEY_Y) {
					return;
				}
				if (hidKeysDown() & KEY_X) {
					break;
				}
			}
			consoleClear();
			std::string temp = u8tostring(firstShake, 8);
			const char * c = temp.c_str();
			std::cout << "   Original: " << temp << std::endl;
			didit = true;
			swkbdInit(&swkbd, SWKBD_TYPE_NORMAL, 3, -1);
			swkbdSetInitialText(&swkbd, mybuf);
			swkbdSetHintText(&swkbd, c);
			strcpy(mybuf, temp.c_str());
			swkbdSetButton(&swkbd, SWKBD_BUTTON_LEFT, "Cancel", false);
			swkbdSetButton(&swkbd, SWKBD_BUTTON_MIDDLE, "Original", true);
			swkbdSetButton(&swkbd, SWKBD_BUTTON_RIGHT, "Modify", true);
			static bool reload = false;
			swkbdSetStatusData(&swkbd, &swkbdStatus, reload, true);
			swkbdSetLearningData(&swkbd, &swkbdLearning, reload, false);
			reload = true;
			button = swkbdInputText(&swkbd, mybuf, sizeof(mybuf));
			if (button == 0)
				return;
			if (button == 1) {
				startTransfer(0, true);
			}
			std::cout << button << std::endl;
			std::cout << mybuf << std::endl;
			//std::copy(secondShake, secondShake + 8, stringtou8(mybuf).data());
			stringtou8(string(mybuf), std::vector<u8>(secondShake, secondShake + sizeof secondShake / sizeof secondShake[0]));
			return;
		}

		std::cout << "   Please turn on the Fit Meter and long press \n   the Fit Meter middle button within 5 seconds." << std::endl;

		u32 receivedSize;
		while (1) {
			if (first == 0) {
				if (data[4] == 0x3 || data[4] == 0x4) {
					if (firstshakedone == 1)
						std::copy(secondShake, secondShake + 8, dataSend);
					printIfError(blockSendData(dataSend, 8));
					firstshakedone++;
				}
			}

			printIfError(blockReceiveData(data, BUFFER_SIZE, &receivedSize, 15000000000));
			if (receivedSize == 0)  {
				std::cout << "   No data detected within 5 seconds. " << std::endl;
				break;
			}

			if (first == 1) {
				if (data[4] == 0x3 || data[4] == 0x4) {
					// Skip 4, because it makes the meter think the connection failed.
					if (firstshakedone == 1)
						std::copy(secondShake, secondShake + 8, dataSend);
					blockSendData(dataSend, 8);
					firstshakedone++;
				}
			}
			if (first == 2) {
				std::copy(data, data + 8, dataSend);
				printIfError(blockSendData(dataSend, receivedSize));
			}
		}
		std::cout << ":: Stopped listening" << std::endl;
	}
	
	void startTransfer2() {
			std::array<u8, 1> firstShake = {0x02};
			std::array<u8, 3> secondShake = {0x00, 0x07, 0xF3};
			std::array<u8, 6> thirdShake = {0x00, 0x10, 0xF4, 0x00, 0x00, 0x00};
			std::array<u8, 6> fourthShake = {0x00, 0x0d, 0xF4, 0x01, 0x00, 0x00};
			std::array<u8, 6> fifthShake = {0x00, 0x1e, 0xF4, 0x02, 0x00, 0x017};
			//std::array<u8, 8> thirdShake = {0xA5, 0x00, 0x03, 0x01, 0x00, 0xF2, 0x00};
			std::array<u8, 1> goodbye = {0x0F};
			const u32 TIMEOUT = 500000000;
			
			const size_t RECEIVE_SIZE = 256;
			std::array<u8, RECEIVE_SIZE> receiveBuffer;
			u32 receivedSize = 0;
			
			std::cout << ":: Emulating Wii U. Please connect a Fit Meter" << std::endl;
			Result res = Link::waitForConnection(10 * TIMEOUT);
			if (R_FAILED(res)) {
				Link::printIfError(res);
				Link::resetConnectionId();
				return;
			}
			std::cout << "   Fit meter found!" << std::endl;
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			Link::blockSendPacket(firstShake, true);
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			Link::blockSendPacket(secondShake, false);
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			Link::blockSendPacket(thirdShake, false);
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			Link::blockSendPacket(fourthShake, false);
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			Link::blockSendPacket(fifthShake, false);
			Link::blockReceivePacket(receiveBuffer, &receivedSize, TIMEOUT);
			
			
			Link::blockSendPacket(goodbye, true);
			Link::resetConnectionId();
		}

	void printIfError(Result ret) {
		if (R_FAILED(ret)) {
			std::cout << "   Result " << std::hex << ret << ": " << std::dec << osStrError(ret) << std::endl;
			u32 level = R_LEVEL(ret);
			std::cout << "   Lvl: " << level << "; Smry: " << R_SUMMARY(ret) << "; Mdl: " << R_MODULE(ret) << "; Desc: " << R_DESCRIPTION(ret) << std::endl;
		}
	}

	void printBytes(u8* bytes, size_t size, bool sender) {
		consoleSelect(&linkConsole);
		if (sender) {
			std::cout << "-> ";
		} else {
			std::cout << "<- ";
			if (pcap->isOpen()) {
				PcapRecord record(size, bytes);
				pcap->addRecord(record);
			}
		}
		
		// Print bytes with a valid CRC in green
		if (crc8_arr(bytes, size - 1) == bytes[size - 1]) {
			std::cout << "\e[32m";
		} else {
			std::cout << "\e[31m";
		}
		
		for (u32 i = 0; i < size; i++){
			std::cout << std::hex << std::setw(2) << std::setfill('0') << (bytes[i] & 0xFF) << " ";
			
			// Add spacing and newlines.
			if ((i + 1) % 4 == 0) std::cout << " ";
			if ((i + 1) % 12 == 0) {
				std::cout << std::endl;
				if ((i + 1) < size) {
					std::cout << "   ";
				}
			}
		}
		std::cout << "\e[0m";
		// Add a newline in case the request wasn't a multiple of 8 bytes.
		if ((size) % 12 != 0) std::cout << std::endl;
		consoleSelect(&defaultConsole);
	}
	
	std::string u8tostring(u8* bytes, size_t size) {
		std::string returnstring;
		std::stringstream buffer;
		for (u32 i = 0; i < size; i++) {
			buffer << std::hex << std::setw(2) << std::setfill('0') << (bytes[i] & 0xFF) << " ";

			if ((i + 1) % 4 == 0) buffer << " ";
		}
		returnstring = buffer.str();
		return returnstring;
	}
	vector<u8> stringtou8(std::string hex, vector<u8> vec) {
		removespace(hex);
		std::vector<string> arr(hex.length() / 2);
		int j = 0;
		for (size_t i = 0; i < hex.length(); i++) {
			arr[i] = hex[j] + hex[j+1];
			j=j+2;
		}

		for (size_t i = 0; i < arr.size(); i++) {
			std::istringstream converter(arr[i]);
			converter >> std::hex >> vec[i];
		}
		return vec;
	}

	std::string removespace(std::string str) {
		str.erase(remove(str.begin(), str.end(), ' '), str.end());
		return str;
	}

	Result initialise() {
		consoleInit(GFX_BOTTOM, &linkConsole);
		consoleInit(GFX_TOP, &defaultConsole);
		buffer = (u8*) memalign(BUFFER_SIZE, BUFFER_SIZE);
		Result ret = 0;
		pcap = new Pcap("fms-" + std::to_string((int) time(nullptr)) + ".pcap");
		if (!pcap->isOpen()) std::cout << ":: Failed to open capture file" << std::endl;
		if (R_FAILED(ret = iruInit( (u32*) buffer, BUFFER_SIZE))) return ret;
		
		// The frequency of the Pokewalker is about 116 279,07 and it looks a lot like a Wii Fit U meter.
		// Let's try that frequency, because you have to do something.
		IRU_SetBitRate(3);
		if (R_FAILED(ret = getRecvFinishedEvent(&recvFinishedEvent))) return ret;
		return ret;
	}
	
	void exit() {
		iruExit();
		free(buffer);
		std::cout << ":: Saving capture file" << std::endl;
		//pcap->flush();
		delete pcap;
	}

	Result getRecvFinishedEvent(Handle* handle) {
		Result ret = 0;
		u32 *cmdbuf = getThreadCommandBuffer();
		
		cmdbuf[0] = IPC_MakeHeader(0xE,0,0); // 0xE0000
		
		if(R_FAILED(ret = svcSendSyncRequest(iruGetServHandle())))return ret;
		
		ret = (Result)cmdbuf[1];
		
		*handle = (Handle) cmdbuf[3];
		
		return ret;
	}
	
	
	Result blockReceiveData(u8* data, size_t size, u32* length, u64 timeout) {
		Result ret = 0;
		if (R_FAILED(ret = IRU_StartRecvTransfer(BUFFER_SIZE, 0))) { 
			std::cout << "StartRecvTransfer failed." << std::endl;
			IRU_WaitRecvTransfer(length);
			return ret;
		}

		svcWaitSynchronization(recvFinishedEvent, timeout);
		if (R_FAILED(ret = svcClearEvent(recvFinishedEvent))) return ret;
		if (R_FAILED(ret = IRU_WaitRecvTransfer(length))) return ret;
		if (*length == 0) {
			return ret;
		}
		std::copy(buffer, buffer + std::min((size_t) *length, size), data);
		
		printBytes(data, (size_t) *length, false);

		return ret;
	}
	
	Result blockReceivePacket(u8* data, size_t size, u32* length, u64 timeout) {
		u8 received[size];
		u32 receivedSize = 0;
		blockReceiveData(received, size, &receivedSize, timeout);
		
		u8 packetSize = 0;
		
		if (receivedSize == 0) {
			std::cout << "   Timeout." << std::endl;
			*length = 0;
			return MAKERESULT(RL_INFO, RS_NOP, RM_LINK, RD_TIMEOUT);
		}
		
		if (receivedSize < 4) {
			// We need at least 4 bytes: Magic number, ConnectionID, Flags/size and a CRC-8.
			*length = 0;
			// Return error if no data was received.
			std::cout << "   Received too few bytes" << std::endl;
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NO_DATA);
		}
		
		if (received[0] != MAGIC) {
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if (received[1] != connectionId && received[1] != 0x00) {
			std::cout << "   Incorrect connectionId" << std::endl;
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if (crc8_arr(received, receivedSize - 1) != received[receivedSize - 1]) {
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if ((received[2] & 0x80) == 0x80 && received[1] != 0x00) {
			// The other client would like to close the connection.
			connectionId = 0;
		}
		
		if ((received[2] & 0x40) == 0x40) {
			// Use the extended size format
			packetSize = received[3];
			memcpy(data, received + 4, packetSize);
		} else {
			// Standard size.
			packetSize = received[2] & 0b00111111;
			memcpy(data, received + 3, packetSize);
		}
		*length = packetSize;
		
		return 0;
	}
	
	Result waitForConnection(u64 timeout) {
		Result ret = 0;
		if (connectionId != 0) {
			// Error if there is already an ongoing connection.
			return MAKERESULT(RL_STATUS, RS_INVALIDSTATE, RM_LINK, RD_ALREADY_EXISTS);
		}
		u8 received[8];
		u32 receivedSize = 0;
		if (R_FAILED(ret = blockReceiveData(received, 8, &receivedSize, timeout))) return ret;
		
		if (receivedSize == 0) {
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NO_DATA);
		}
		
		if (receivedSize != 8) {
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if (received[0] != MAGIC && received[1] != 00) {
			// This probably wasn't meant for us.
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if (crc8_arr(received, 7) != received[7]) {
			// Checksum doesn't match. Probably an invalid packet.
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NOT_FOUND);
		}
		
		if (received[3] != 0x01 || received[4] != 0x03 || received[5] != 0x04) {
			// There is no intention to start a connection.
			return MAKERESULT(RL_STATUS, RS_NOP, RM_LINK, RD_NO_DATA);
		}
		
		connectionId = received[6];
		return ret;
	}
	
	Result blockSendData(u8* data, u32 length) {
		data[length - 1] = crc8_arr(data, length - 1);
		Result ret = 0;
		if (R_FAILED(ret = IRU_StartSendTransfer(buffer, length))) return ret;
		IRU_WaitSendTransfer();
		
		printBytes(data, length, true);
		return ret;
	}
	
	Result blockSendPacket(u8* data, u32 length, bool close) {
		u8* packet;
		u32 packetSize;
		if (length > 0b00111111) {
			// We are using the large packet format.
			packetSize = length + 5;
			packet = new u8[packetSize];
			packet[2] = 0x40;
			packet[3] = length;
			memcpy(packet + 4, data, length);
		} else {
			packetSize = length + 4;
			packet = new u8[packetSize];
			packet[2] = length;
			memcpy(packet + 3, data, length);
		}
		packet[0] = MAGIC;
		packet[1] = connectionId;
		if (close) packet[2] |= 0x80;
		packet[packetSize - 1] = crc8_arr(packet, packetSize - 1);
		
		Result res = blockSendData(packet, packetSize);
		delete[] packet;
		return res;
	}
	
	void resetConnectionId() {
		connectionId = 0;
	}
}
