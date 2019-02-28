#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <iostream>
#include <3ds.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <iomanip>
#include <string.h>
#include "crc.h"
#include <algorithm>
#include <vector>

typedef u8 uint8_t;

using namespace std;

namespace FMS {
    void receiveData();
    void startTransfer(int first, bool editdata);
    void printIfError(Result result);
    void printBytes(u8* bytes, size_t size, bool sender);
	std::string u8tostring(u8* bytes, size_t size);
	vector<u8> stringtou8(std::string hex, vector<u8> vec);
	std::string removespace(std::string str);
    extern u8 curByte;
    extern u8 curByte2;
	
	Result getRecvFinishedEvent(Handle* handle);
}

#endif //LINK_H
