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

namespace FMS {
    void receiveData();
    void startTransfer();
    void printIfError(Result result);
    void printBytes(u8* bytes, size_t size, bool sender);
    extern u8 curByte;
    extern u8 curByte2;
	
	Result getRecvFinishedEvent(Handle* handle);
}

#endif //LINK_H
