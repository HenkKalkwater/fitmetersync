#include "link.h"

namespace FMS::Link {
    u8* buffer;
    Handle recvFinishedEvent = 0;
	PrintConsole linkConsole;
	PrintConsole defaultConsole;
	std::ofstream outStream;

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
            
            printIfError(blockReceiveData(data, &receivedSize, 15000000000));
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

    void printIfError(Result ret) {
        if (ret != 0) {
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
			// Also print to file
			outStream << "---" << std::endl;
			for (size_t i = 0; i < size; i++) {
				outStream << std::hex << std::setw(2) << std::setfill('0') << (bytes[i] & 0xFF) << " ";
			}
			outStream << std::endl;
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
            if ((i + 1) % 8 == 0) {
				std::cout << std::endl;
				if ((i + 1) < size) {
					std::cout << "   ";
				}
			}
        }
        std::cout << "\e[0m";
        // Add a newline in case the request wasn't a multiple of 8 bytes.
        if ((size) % 8 != 0) std::cout << std::endl;
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
		outStream.open("fms-" + std::to_string((int) time(nullptr)) + ".txt");
		if (!outStream.is_open()) std::cout << ":: Failed to open log file" << std::endl;
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
		outStream.close();
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
    
    
    Result blockReceiveData(u8* data, u32* length, u64 timeout) {
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
        //std::copy(buffer.begin(), buffer.begin() + *length, data);
        for (size_t i = 0; i < *length; i++) {
            data[i] = buffer[i];
        }
        
        printBytes(data, (size_t) *length, false);
        
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
}
