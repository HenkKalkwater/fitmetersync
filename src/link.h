#ifndef LINK_H
#define LINK_H

#include <cstdint>
#include <iostream>
#include <fstream>
#include <3ds.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>
#include <iomanip>
#include <string.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <iterator>

#include "crc.h"
#include "pcap.h"
typedef u8 uint8_t;

using namespace std;

namespace FMS::Link {
	/**
	 * \brief Result Module that is used for results caused by the functions in this namespace.
	 */
	const u8 RM_LINK = 253;

	/**
	 * \brief Magic number at the start of each packet.
	 */
	const u8 MAGIC = 0xA5;

	/**
	 * \brief Id of the active connection, 0 if not in a connection.
	 */
	extern u8 connectionId;
	
	void startTransfer(int first, bool editdata);
	void startTransfer2();

	/**
	 * \brief Prints information to the screen if a result wasn't successful.
	 */
	void printIfError(Result result);

	std::string u8tostring(u8* bytes, size_t size);
	vector<u8> stringtou8(std::string hex, vector<u8> vec);
	std::string removespace(std::string str);

    
	extern PrintConsole defaultConsole;
	extern PrintConsole linkConsole;

	extern std::ofstream outStream;
    extern Pcap* pcap;

	/**
	 * \brief The size of the buffer used to receiving and sending data.
	 */
	const size_t BUFFER_SIZE = 0x1000;
	/**
	 * \brief Buffer used for receiving data.
	 */
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
	 * \param[in] size The size of the array where the data should be copied.
	 * \param[out] length A pointer which will be the length of the data received.
	 * \param[in] timeout The maximum amount of time to wait, in nanoseconds.
	 * \returns Result describing wether this call was succesful.
	 * 
	 * Blocks the thread until data over IR has been received. If a timeout happens, it will complete
	 * the transfer. Length is set to the total amount of bytes received. If the IR module receives more bytes
	 * than fit in the \c data , it will shrink to the size of \c data , but the length will still be
	 * the actual bytes received.
     */
	Result blockReceiveData(u8* data, size_t size, u32* length, u64 timeout=U64_MAX);
	
	/**
	 * \brief Waits and blocks the thread until data over IR has been received.
	 * \param[out] data A pointer of the array to copy the data inside the to. (Does not copy the whole packet!)
	 * \param[in] size The size of the array.
	 * \param[out] length The size of the bytes received.
	 * \param[in] timeout The maximum amount of time to wait, in nanoseconds.
	 * \see #blockReceiveData
	 * \returns Success if a valid packet was received, an error if an invalid packet was received of the IR communication failed.
	 * 
	 * Waits and blocks the thread until a packet over IR has been received. This will validate the packet and throw away invalid ones.
	 */
	Result blockReceivePacket(u8* data, size_t size, u32* length, u64 timeout=U64_MAX);
	
	template<std::size_t SIZE>
	Result blockReceivePacket(std::array<u8, SIZE>& data, u32* length, u64 timeout=U64_MAX) {
		return blockReceivePacket(data.data(), data.size(), length, timeout);
	}
	
	/**
	 * \brief Waits for a connection.
	 * \returns Result describing whether the attempt was succesful.
	 */
	Result waitForConnection(u64 timeout=U64_MAX);
	
	/**
	 * \brief Calculates checksum and sends data over IR. Will block the thread until the data has 
	 * has been sent.
	 * \param[in] data A pointer to the array which should be sent.
	 * \param[in] length The length of the data + 1 (for the checksum).
	 * \result Result describing wether this call was succesful.
	 */
	Result blockSendData(u8* data, u32 length);
	
	template<std::size_t SIZE>
	Result blockSendData(std::array<u8, SIZE>& data) {
		return blockSendData(data.data(), data.size());
	};
	
	/**
	 * \brief Creates a packet with the data and adds the necessary information.
	 * \param[in] data The data to send.
	 * \param[in] length The lenght of the data
	 * \param[in] close If the connection should be closed after this packet.
	 */
	Result blockSendPacket(u8* data, u32 length, bool close=false);
	
	template<std::size_t SIZE>
	Result blockSendPacket(std::array<u8, SIZE>& data, bool close=false) {
		return blockSendPacket(data.data(), data.size(), close);
	};
	
	/**
	 * Resets the connection ID.
	 */
	void resetConnectionId();
}

#endif //LINK_H
