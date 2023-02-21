// SPDX-License-Identifier: GPL-3.0-or-later
#include <algorithm>

namespace fms {
namespace protocol {

template <class It>
LinkPacket::LinkPacket(uint8_t connectionId, It payloadBegin, It payloadEnd)
	: m_connectionId(connectionId) {
	m_payload.resize(std::distance(payloadBegin, payloadEnd));
	std::copy(payloadBegin, payloadEnd, m_payload.begin());
	recalculateFields();
}

template <class It>
void LinkPacket::setPayload(It payloadBegin, It payloadEnd) {
	m_payload.resize(std::distance(payloadBegin, payloadEnd));
	std::copy(payloadBegin, payloadEnd, m_payload.begin());
	recalculateFields();
}

template <class It>
std::size_t LinkConnection::receive(It start, It end) {
	auto packet = receivePacket();
	std::copy(packet.payload().begin(), packet.payload().end(), start);
	return packet.payload().size();
}

} // NS protocol
} // NS fms
