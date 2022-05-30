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
#include "link.h"
#include "tsdeque.h"

namespace fms {
	
	enum PacketType {
		NONE = 0x0,
		REPLY_FLAG = 0x1,
		SEND_RAW = 0x2,
		SEND_RAW_REPLY = 0x3,
		RECEIVE_RAW = 0x4,
		RECEIVE_RAW_REPLY = 0x5,
		WAIT_CONNECTION = 0x6,
		WAIT_CONNECTION_REPLY = 0x7,
		SEND_PACKET = 0x8,
		SEND_PACKET_REPLY = 0x9,
		RECEIVE_PACKET = 0xA,
		RECEIVE_PACKET_REPLY = 0xB
	};
	
	class Packet {
	public:
		Packet(PacketType type, std::optional<std::vector<u8>> payload = std::nullopt) : m_type(type), m_payload(payload) {}
		virtual ~Packet() {}
		PacketType type() const { return m_type; }
		static std::optional<Packet*> fromVector(const std::vector<u8> &bytes);
		std::optional<std::vector<u8>> payload() { return m_payload; }
		
		bool isReply() const { return m_type & PacketType::REPLY_FLAG; }
		virtual std::vector<u8> asVector();
	protected:
		const PacketType m_type;
		const std::optional<std::vector<u8>> m_payload;
	};
	
	class ReplyPacket : public Packet {
	public:
		ReplyPacket(PacketType type, Result result, std::optional<std::vector<u8>> payload = std::nullopt) : Packet(type, payload), m_result(result){}
		Result result() const { return m_result; }
		std::vector<u8> asVector() override;
	private:
		const Result m_result;
	};
	
	const size_t PACKET_HEADER_SIZE = 8;
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
		PacketType nextType = NONE;
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
		static void handleIrComms(void* args);
		void updateState(SocketState &state, u8* data, size_t newDataLength);
		
		Thread acceptThread;
		Thread irComsThread;
		
		struct handleClientArgs {
			int client_socket;
		};
		int server_socket;
		bool hasConnection = false;
		bool requestedStop = false;
		/**
		 * Packets still to be sent to the computer.
		 * Note: the consumer is responsible for deleting the pointers.
		 */
		TSDeque<Packet*> pendingIPPackets;
		/**
		 * Packets still needed to be sent over IR.
		 * Note: the consumer is responsible for deleting the pointers.
		 */
		TSDeque<Packet*> pendingIRPackets;
		
		Handle ev_stop, ev_received, ev_send;
	};
	
}
#endif //RESERVER_H
