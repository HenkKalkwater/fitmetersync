// SPDX-License-Identifier: GPL-3.0-or-later

#include "../iradapter.h"

#include "../iradaptermanager.h"


namespace fms {
namespace platform {

IRAdapter::IRAdapter(const IRAdapterDescriptor &desc)
	: d_ptr(new IRAdapterPrivate) {
	// TODO: check if opening was successfull
	d_ptr->m_fileNo = open(desc.m_handle.c_str(), O_RDWR);

	// Set up tty options
	// TODO: check if tcgetattr was successfull
	tcgetattr(d_ptr->m_fileNo, &d_ptr->m_tty);
	cfsetispeed(&d_ptr->m_tty, B115200);
	cfsetospeed(&d_ptr->m_tty, B115200);
	d_ptr->m_tty.c_cflag &= ~PARENB;
	d_ptr->m_tty.c_cflag &= ~CSIZE;
	d_ptr->m_tty.c_cflag |= CS8; // 8 bits per byte
	d_ptr->m_tty.c_lflag &= ~ICANON; // Disable canonical mode (waiting for newline)
	d_ptr->m_tty.c_lflag &= ~ECHO;
	d_ptr->m_tty.c_lflag &= ~ECHOE;
	d_ptr->m_tty.c_lflag &= ~ECHONL;
	d_ptr->m_tty.c_lflag &= ~ISIG;
	d_ptr->m_tty.c_lflag &= ~OPOST;
	// TODO: check if tcsetattr was successfull
	tcsetattr(d_ptr->m_fileNo, TCSANOW, &d_ptr->m_tty);
	tcflush(d_ptr->m_fileNo, TCIOFLUSH);
}


IRAdapter::IRAdapter(IRAdapter &&other)
	: d_ptr(std::move(other.d_ptr)) {}

IRAdapter::~IRAdapter() {}
} // NS platform
} // NS fms
