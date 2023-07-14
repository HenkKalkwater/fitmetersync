// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PROTOCOL_LINK_H
#define FMS_PROTOCOL_LINK_H

#include <array>
#include <core/types.h>

#include <stdint.h>

#include <optional>
#include <vector>

namespace fms {

namespace platform {
class IRAdapter;
} // NS platform

namespace protocol {

class IRLinkException : public std::exception {
public:
	explicit IRLinkException(uint32_t errorCode);

	virtual const char *what() const noexcept override;

	const static uint32_t ERR_IR_LINK = 0x01'00'00'00;
	const static uint32_t ERR_IR_LINK_CORRUPT_PACKET = ERR_IR_LINK | 0x01;
	const static uint32_t ERR_IR_LINK_INVALID_STATE  = ERR_IR_LINK | 0x02;
	const static uint32_t ERR_IR_LINK_CLOSED         = ERR_IR_LINK | 0x03;
private:
	uint32_t errorCode;
};

class LinkConnection;

/**
 * \brief Packet on the link layer
 *
 *
 * # Byte layout:
 *
 *| Name         | Offset (bytes)                            | Length (bytes) | Value description                                                                 |
 *|--------------|-------------------------------------------|----------------|-----------------------------------------------------------------------------------|
 *| Magic        | 0x0                                       | 0x1            | Always `a5`?                                                                      |
 *| ConnectionID | 0x1                                       | 0x1            | Connection ID, `00` if there is no connection made yet. (?)                       |
 *| SetupFlag    | 0x2                                       | 0x1            | First bit: 1 if payload contains data relevant for the connection instead of an actual payload |
 *| LargeFlag    | ^                                         | ^              | Second bit: 1 if the payload size is larger than 0x3F and therefore the size is stored in the next byte instead |
 *| PacketSize   | ^                                         | ^              | Remaing 6 bits: length of the `Data` (in bytes) if `LargeFlag` is not set.          |
 *| PacketSize   | 0x3 (Only present if `LargeFlag` was set) | 0x1            | Size (in the case that `LargeFlag` was set, otherwise data starts here.)          |
 *| Payload      | 0x3 (0x4 if `LargeFlag` was set)          | Y              | Data (see below)                                                                  |
 *| Checksum     | 0x3 + Y (0x4 + Y if `LargeFlag` was set)  | 0x1            | 8-bit CRC over all the previous bytes                                             |
 *
 * ## Payload when setup flag is set
 * | Name         | Offset (bytes) | Length (bytes) | Value description                               |
 * |--------------|----------------|----------------|-------------------------------------------------|
 * | Command      | 0x0            | 0x1            | `0x01` to "advertise" a connection              |
 * | ^            | ^              | ^              | `0x02` to reply to a connection advertisement   |
 * | ^            | ^              | ^              | `0x0F` to close the current connection          |
 *
 * If the `Command` is `0x01`, the payload contains the following data as well.
 *
 * | Name         | Offset (bytes) | Length (bytes) | Value description                                           |
 * |--------------|----------------|----------------|-------------------------------------------------------------|
 * | Unknown      | 0x1            | 0x2            | Unknown. The fit meter always sends `0x03 0x04`             |
 * | ConnectionId | 0x3            | 0x1            | The ConnectionID to use when replying to this advertisement |
 */
class LinkPacket {
	friend class LinkConnection;
public:
	enum SetupState : int8_t {
		/// The connection should be opened (additional data seems to be send)
		CONNECTION_OPEN     = 0x01,
		/// The connection should be opened
		CONNECTION_OPEN_ACK = 0x02,
		/// The connection should be closed.
		CONNECTION_CLOSE    = 0x0F
	};

	/// \brief ConnectionId that accepts any connection
	static const uint8_t CONNECTION_ID_ANY = 0x00;



	/**
	 * \brief Creates an empty packet
	 */
	LinkPacket(uint8_t connectionId);

	/**
	 * \brief Creates a packet with the given payload
	 * \param payloadBegin Begin iterator of the payload data
	 * \param payloadEnd End iterator of the payload data
	 */
	template <class It>
	LinkPacket(uint8_t connectionId, It payloadBegin, It payloadEnd);

	static LinkPacket createOpenPacket();
	static LinkPacket createOpenAckPacket(uint8_t connectionId);
	static LinkPacket createClosePacket(uint8_t connectionId);

	/// \brief Retrieves the data in this vector
	const std::vector<u8> &payload() const;
	u8 connectionId() const;
	bool isSetupPacket() const;
	u8 checksum() const;

	/**
	 * \brief Copies the packet wire representation into a vector
	 * \note If the packet is created in the code, you may need to call recalculateFields() before
	 *       getting a correct output.
	 */
	std::vector<u8> serializePacket() const;

	/**
	 * \brief Sets the payload
	 * \param payloadBegin Begin iterator of the payload data
	 * \param payloadEnd End iterator of the payload data
	 */
	template <class It>
	void setPayload(It payloadBegin, It payloadEnd);
private:
	/// \brief Size before the packet gets split
	static const uint8_t PACKET_LARGE_THRESHOLD = 0x3F;

	LinkPacket() {}
	/// \brief Recalculates the checksum and size fields.
	void recalculateFields();
	uint8_t calculateCrc() const;

	struct FlagsAndSize {
		uint8_t size : 6;
		bool large: 1;
		bool setup : 1;
	};

	static_assert(sizeof(FlagsAndSize) == 1);

	static const uint8_t MAGIC = 0xA5;
	bool m_setupFlag = false;
	uint8_t m_connectionId = 0;
	std::vector<uint8_t> m_payload;
	uint8_t m_checksum = 0;
};

/**
 * \brief State machine for a connection.
 */
class LinkConnection {
public:
	explicit LinkConnection(platform::IRAdapter &adapter);

	/**
	 * \brief Waits for and opens a connection with the specified connection ID
	 * \param connectionId The id of the connection to accept. Use LinkPacket::CONNECTION_ID_ANY to accept any connection.
	 * \return True if a connection has been accepted, false otherwise.
	 */
	bool accept(uint8_t connectionId = LinkPacket::CONNECTION_ID_ANY);

	/**
	 * \brief Closes the current connection
	 * The connection to close.
	 */
	void close();

	/**
	 * \brief Sends data over the link
	 */
	template <class It>
	void send(It start, It end);

	/**
	 * \brief Receives data over the link
	 *
	 * \return The amount of bytes read.
	 */
	template <class It>
	std::size_t receive(It start, It end);

	/**
	 * \brief Tries to receive a packet, but retains all
	 * metadata of the encapsulating packet.
	 */
	LinkPacket receivePacket();
private:
	void sendPacket(const LinkPacket &packet);

	platform::IRAdapter &m_adapter;
	bool m_connectionOpen = false;
	uint8_t m_connectionId;
	std::array<u8, 0xFF> m_buffer;
	size_t m_bufferIndex = 0;
};

} // NS protocol
} // NS fms

#include "link.inl"

#endif // FMS_PROTOCOL_LINK_H
