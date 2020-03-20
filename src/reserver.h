#ifndef RESERVER_H
#define RESERVER_H

#include <3ds.h>

#include <array>
#include <iostream>
#include <optional>
#include <queue>
#include <vector>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <netinet/in.h>

#include "debug.h"
#include "tsdeque.h"

namespace FMS {
	enum PacketType {
		NONE,
		SEND_RAW,
		SEND_RAW_REPLY,
		RECEIVE_RAW,
		RECEIVE_RAW_REPLY,
		WAIT_CONNECTION,
		WAIT_CONNECTION_REPLY,
		SEND_PACKET,
		SEND_PACKET_REPLY,
		RECEIVE_PACKET,
		RECEIVE_PACKET_REPLY
	};
	/**
	 * Each packet has the very simple structure of:
	 * PacketType type
	 * u32 dataLength
	 * u8[dataLength] payload
	 * 
	 * The payload for send_raw and send_packet is the payload to send.
	 * The payload for receive_raw_reply and receive_packet_reply is an Result, followed by the payload
	 * The payload for send_raw_reply, wait_connection_reply, send_packet_reply is an Result
	 * The payload for receive_raw, wait_connection and receive_packet is empty.
	 * 
	 * This struct is used for keeping the state machine of parsing the data.
	 */
	struct SocketState {
		PacketType nextType;
		u32 expectedPacketSize = 0;
		std::vector<u8> buffer;
	};
	
	/**
	 * Reverse-Engineer Server. Used to send received IR data to a connected computer,
	 * and to send received packets back over IR.
	 */
	class REServer {
	public:
		REServer();
		/**
		 * Opens the socket and starts listening. This blocks until the socket
		 * is established.
		 * \returns True if opening was successfull.
		 */
		bool open();
		void sendPacket(std::vector<u8> packet) {}
		std::vector<u8> receivePacket();
		void close();
	private:
		/**
		 * Gets called when a new connection has been made.
		 */
		void handleClient(void* args);
		static void handleAccept(void* args);
		void updateState(SocketState &state, u8* data, size_t newDataLength);
		
		Thread acceptThread;
		
		struct handleClientArgs {
			int client_socket;
		};
		int server_socket;
		bool hasConnection = false;
		bool requestedStop = false;
		/**
		 * Packets still to be sent to the computer.
		 */
		TSDeque<std::vector<u8>> pendingIPPackets;
		/**
		 * Packets still needed to be sent over IR.
		 */
		TSDeque<std::vector<u8>> pendingIRPackets;
		
		Handle ev_stop, ev_received, ev_send;
	};
	
}
#endif //RESERVER_H
