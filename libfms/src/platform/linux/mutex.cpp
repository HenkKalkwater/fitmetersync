// SPDX-License-Identifier: GPL-3.0-or-later

#include "../mutex.h"

namespace fms {
namespace platform {


Mutex::Mutex() : m_lock(std::mutex()) {

}

void Mutex::lock() {
	m_lock.lock();
}

bool Mutex::try_lock() {
	return m_lock.try_lock();
}

void Mutex::unlock() {
	m_lock.unlock();
}

}
}
