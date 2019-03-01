#include "wiiulink.h"

namespace FMS {
	void WiiULink::test() {
		std::array<u8, 5> firstShake = {0xA5, 0x00, 0x81, 0x02, 0x00};
		std::array<u8, 7> secondShake = {0xA5, 0x00, 0x02, 0x03, 0x07, 0xF3, 0x00};
		//std::array<u8, 8> thirdShake = {0xA5, 0x00, 0x03, 0x01, 0x00, 0xF2, 0x00};
		std::array<u8, 5> goodbye = {0xA5, 0x00, 0x81, 0x0f, 0x00};
		std::array<u8, 256> receiveBuffer;
		u32 receivedSize = 0;
		u32 connectionNumber = 0;
		
		Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 15000000000);
		if (receivedSize > 7) {
			connectionNumber = receiveBuffer[6];
			firstShake[1] = connectionNumber;
			secondShake[1] = connectionNumber;
			//thirdShake[1] = connectionNumber;
			goodbye[1] = connectionNumber;
		}
		Link::blockSendData(firstShake.data(), firstShake.size());
		//Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 15000000000);
		Link::blockSendData(secondShake.data(), secondShake.size());
		//Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 500000000);
		//Link::blockSendData(secondShake.data(), secondShake.size());
		//Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 500000000);
		//Link::blockSendData(thirdShake.data(), secondShake.size());
		Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 15000000000);
		Link::blockSendData(goodbye.data(), goodbye.size());
		Link::blockReceiveData(receiveBuffer.data(), &receivedSize, 1500000000);
		
		{
			//for (u8 i = 0; i < 0xFF; i++) {
				//secondShake[6] = i;
				//Link::blockSendData(secondShake.data(), secondShake.size());
			//}
		}
	}
}
