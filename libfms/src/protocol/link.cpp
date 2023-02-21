// SPDX-License-Identifier: GPL-3.0-or-later
#include "link.h"

#include <core/debug.h>
#include <core/crc.h>
#include <platform/iradapter.h>

#include <array>
#include <algorithm>

namespace fms {
namespace protocol {

IRLinkException::IRLinkException(uint32_t errorCode) : errorCode(errorCode) {}
const char * IRLinkException::what() const noexcept {
	switch(errorCode) {
	case ERR_IR_LINK_CORRUPT_PACKET:
		return "Received corrupt packet";
	case ERR_IR_LINK_INVALID_STATE:
		return "Link is in invalid state";
	case ERR_IR_LINK_CLOSED:
		return "Link is closed";
	}
	return "Unknown error";
}



void LinkPacket::recalculateFields() {
	m_checksum = calculateCrc();
}

uint8_t LinkPacket::calculateCrc() const {
	uint8_t crc = 0;
	crc8(crc, MAGIC);
	crc8(crc, m_connectionId);

	union {
		FlagsAndSize flags;
		uint8_t flagsByte;
	};

	flags.setup = m_setupFlag;
	uint8_t payloadSize = m_payload.size();

	if (payloadSize > PACKET_LARGE_THRESHOLD) {
		flags.large = true;
		flags.size = 0;
		crc8(crc, flagsByte);
		crc8(crc, payloadSize);
	} else {
		flags.large = false;
		flags.size = payloadSize;
		crc8(crc, flagsByte);
	}
	crc = crc8_arr(m_payload.data(), m_payload.size(), crc);
	return crc;
}

const std::vector<u8>& LinkPacket::payload() const {
	return m_payload;
}

u8 LinkPacket::connectionId() const {
	return m_connectionId;
}

bool LinkPacket::isSetupPacket() const {
	return m_setupFlag;
}

u8 LinkPacket::checksum() const {
	return m_checksum;
}

// LinkConnection

LinkConnection::LinkConnection(platform::IRAdapter& adapter)
	: m_adapter(adapter) {

}

bool LinkConnection::accept(uint8_t connectionId) {
	if (m_connectionOpen) throw IRLinkException(IRLinkException::ERR_IR_LINK_INVALID_STATE);
	for (int attempts  = 3; attempts > 0; attempts--) {
		FMS_INF("Accepting connection, attempt %d", attempts);

		LinkPacket packet;
		try {
			packet = LinkConnection::receivePacket();
		} catch(std::exception *e) {
			FMS_WRN("Uncaught exception: %s", e->what());
		} catch(...) {
			FMS_INF("Unknown uncaught exception");
			continue;
		}

		if (!packet.isSetupPacket()) {
			FMS_DBG("Not a setup packet");
			continue;
		}
		std::vector<uint8_t> payload = std::move(packet.m_payload);
		FMS_HEXDMP(payload.begin(), payload.end());
		if (payload.size() != 4) continue;
		if (payload[0] != LinkPacket::CONNECTION_OPEN) continue;

		if (connectionId == LinkPacket::CONNECTION_ID_ANY
			// TODO: verify the assumption made below
			|| connectionId == payload[3]) {
			m_connectionId = payload[3];
			m_connectionOpen = true;
		}

		std::array<uint8_t, 1> replyPayload = { LinkPacket::CONNECTION_OPEN_ACK };
		LinkPacket replyPacket(m_connectionId, replyPayload.begin(), replyPayload.end());
		replyPacket.m_setupFlag = true;
		replyPacket.recalculateFields();
		sendPacket(replyPacket);
		FMS_DBG("Link connnected, id = %d", m_connectionId);
		return true;
	}
	return false;
}

void LinkConnection::close() {
	if (!m_connectionOpen) throw IRLinkException(IRLinkException::ERR_IR_LINK_INVALID_STATE);

	std::array<uint8_t, 1> payload = { LinkPacket::CONNECTION_CLOSE };
	LinkPacket packet(m_connectionId, payload.begin(), payload.end());
	packet.m_setupFlag = true;
	packet.recalculateFields();
	sendPacket(packet);

	m_connectionOpen = false;
	m_connectionId = 0;
}



LinkPacket LinkConnection::receivePacket() {
	m_bufferIndex = 0;
	// Get at least 4 bytes in the buffer, so we can know the length for sure.
	while (m_bufferIndex < 4) {
		std::size_t length;
		m_adapter.read(m_buffer.begin() + m_bufferIndex, m_buffer.end(), length);
		m_bufferIndex += length;
	}

	if (m_buffer[0] != LinkPacket::MAGIC) {
		throw IRLinkException(IRLinkException::ERR_IR_LINK_CORRUPT_PACKET);
	}

	LinkPacket::FlagsAndSize flagsAndSize = *reinterpret_cast<LinkPacket::FlagsAndSize*>(&m_buffer[2]);

	uint8_t packetLength;
	decltype(m_buffer)::iterator payloadStart;
	if (flagsAndSize.large) {
		packetLength = m_buffer[3];
		payloadStart = m_buffer.begin() + 4;
	} else {
		packetLength = flagsAndSize.size;
		payloadStart = m_buffer.begin() + 3;
	}


	while (m_buffer.size() < packetLength) {
		std::size_t length;
		m_adapter.read(m_buffer.begin() + m_bufferIndex, m_buffer.end(), length);
		m_bufferIndex += length;
	}

	LinkPacket result;
	result.m_payload = std::vector<uint8_t>(payloadStart, m_buffer.begin() + m_bufferIndex - 1);
	result.m_setupFlag = flagsAndSize.setup;
	result.m_connectionId = m_buffer[1];
	result.m_checksum = m_buffer[m_bufferIndex - 1];

	FMS_DBG("Packet received, connection %d, setup: %b, large: %b, size: %d", result.connectionId(), result.isSetupPacket(), flagsAndSize.large, packetLength);
	FMS_HEXDMP(m_buffer.begin(), m_buffer.begin() + m_bufferIndex);
	FMS_DBG("Checksum %x vs expected %x", result.m_checksum, result.calculateCrc());

	std::fill(m_buffer.begin(), m_buffer.end(), 0);
	m_bufferIndex = 0;
	return result;
}

void LinkConnection::sendPacket(const fms::protocol::LinkPacket& packet) {
	bool largePacket = packet.payload().size() > packet.PACKET_LARGE_THRESHOLD;
	std::array<u8, 4> header = {LinkPacket::MAGIC, 0, 0, 0};
	header[1] = packet.connectionId();
	LinkPacket::FlagsAndSize *flagsAndSize = reinterpret_cast<LinkPacket::FlagsAndSize *>(&header[2]);
	flagsAndSize->large = largePacket;
	flagsAndSize->setup = packet.isSetupPacket();

	if (largePacket) {
		header[3] = packet.payload().size();
		m_adapter.send(header.begin(), header.end());
	} else {
		flagsAndSize->size = packet.payload().size();
		m_adapter.send(header.begin(), header.end() - 1);
	}
	m_adapter.send(packet.payload().begin(), packet.payload().end());

	u8 crc = packet.checksum();
	m_adapter.send(&crc, &crc + 1);
}



}
} // NS fms
