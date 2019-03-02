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
#include <sstream>
#include <iterator>

typedef u8 uint8_t;

using namespace std;

namespace FMS::Link {
    void receiveData();
    void startTransfer(int first, bool editdata);
    void printIfError(Result result);
	void MITMattack();
    void mimicWiiU();
    
	std::string u8tostring(u8* bytes, size_t size);
	vector<u8> stringtou8(std::string hex, vector<u8> vec);
	std::string removespace(std::string str);
    
    extern u8 curByte;
    extern u8 curByte2;
    
    /**
     * \brief The size of the buffer used to receiving and sending data.
     */
    const size_t BUFFER_SIZE = 0x1000;
    extern u8* buffer;
    
    /**
     * \brief A handle to the receivedFinishedEvent. Used for internal methods.
     */
    extern Handle recvFinishedEvent;
    
    /**
     * \brief Initialises the link. Call this before using any of the methods.
     * \returns Result describing wether this call was successful.
     * 
     * This will acquire the \c ir:u service and initialise some variables needed for the
     * connection.
     */
    Result initialise();
    
    /**
     * \brief Frees memory and cleans up other stuff.
     */
    void exit();
	
    /**
     * \brief Gets the event which is called when data is received over IR.
     * \param[out] handle A pointer to the handle of the event.
     * \returns A result describing if this call was successful.
     * \movetolibctru
     */
	Result getRecvFinishedEvent(Handle* handle);
    
    /**
     * \brief Prints bytes in their hexidecimal representation on the screen.
     * \param[in] bytes A pointer to an array of bytes to print.
     * \param[in] size The size of the array of bytes.
     * \param[in] sender True if this data was sent by the 3DS, false if the data was received.
     */
    void printBytes(u8* bytes, size_t size, bool sender);
    
    /**
     * \brief Waits and blocks the thread until data over IR has been received.
     * \param[out] data A pointer of the array to copy data to.
     * \param[out] length A pointer which will be the length of the data received.
     * \param[in] timeout The maximum amount of time to wait, in nanoseconds (Def)
     * \result Result describing wether this call was succesful.
     */
    Result blockReceiveData(u8* data, u32* length, u64 timeout=U64_MAX);
    
    /**
     * \brief Calculates checksum and sends data over IR. Will block the thread until the data has 
     * has been sent.
     * \param[in] data A pointer to the array which should be sent.
     * \param[in] length The length of the data + 1 (for the checksum).
	 * \param[in] if it should recalculate the last bit.
     * \result Result describing wether this call was succesful.
     */
    Result blockSendData(u8* data, u32 length, bool calcCRC8);
}

#endif //LINK_H
