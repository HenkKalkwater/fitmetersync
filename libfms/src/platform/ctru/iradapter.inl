// SPDX-License-Identifier: GPL-3.0-or-later

#include <3ds.h>

#include <algorithm>
#include <array>
#include <memory>

namespace fms {
namespace platform {

/// \internal
struct IRAdapterPrivate {
	// Mapped read-only for this process, handed over to the IR service
	std::unique_ptr<std::array<u32, 0x1000>> buffer;
	// Event handle for when the IR service has received data
	Handle receiveEvent;
};

template<class It>
It IRAdapter::read(It outBufStart, It outBufEnd, std::size_t &readLength) {
	auto distance = std::distance(outBufStart, outBufEnd);

	Result res = ::IRU_StartRecvTransfer(distance, 0);
	if (R_FAILED(res)) {
		throw std::exception();
	}

	u32 sendCount;
	res = ::IRU_WaitRecvTransfer(&sendCount);
	readLength = sendCount;
	if (R_FAILED(res)) {
		throw std::exception();
	}
	return std::copy(d_ptr->buffer->begin(), d_ptr->buffer->begin() + distance, outBufStart);
}

template<class It>
void IRAdapter::send(It inBufStart, It inBufEnd) {
	auto distance = std::distance(inBufStart, inBufEnd);
	u8 sendBuf[distance];
	std::copy(inBufStart, inBufEnd, sendBuf);

	Result res = ::IRU_StartSendTransfer(sendBuf, distance);
	if (R_FAILED(res)) {
		throw std::exception();
	}
	res = ::IRU_WaitSendTransfer();
	if (R_FAILED(res)) {
		throw std::exception();
	}
}

} // NS platform
} // NS fms
