// SPDX-License-Identifier: GPL-3.0-or-later

#include <core/debug.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <vector>

namespace fms {
namespace platform {

struct IRAdapterPrivate {
	int m_fileNo;
	termios m_tty;
};

template<class It>
It IRAdapter::read(It outBufStart, It outBufEnd, std::size_t &readLength) {
	size_t distance  = std::distance(outBufStart, outBufEnd);
	std::vector<u8> recvBuf = std::vector<u8>(distance);

	ssize_t nread = ::read(d_ptr->m_fileNo, recvBuf.data(), distance);

	if (nread > 0) {
		readLength = nread;
		return std::copy(recvBuf.begin(), recvBuf.begin() + distance, outBufStart);
	} else {
		throw std::exception();
	}
}

template<class It>
void IRAdapter::send(It inBufStart, It inBufEnd) {
	size_t distance  = std::distance(inBufStart, inBufEnd);

	std::vector<u8> sendBuf = std::vector<u8>(distance);
	std::copy(inBufStart, inBufEnd, sendBuf.begin());
	FMS_HEXDMP(inBufStart, inBufEnd);

	ssize_t nwrite = 0;
	while (nwrite < static_cast<ssize_t>(distance)) {
		ssize_t res = ::write(d_ptr->m_fileNo, sendBuf.data(), distance);
		if (res >= 0) {
			nwrite += res;
		} else {
			throw std::exception();
		}
	}
}

} // platform
} // fms
