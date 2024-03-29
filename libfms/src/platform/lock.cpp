// SPDX-License-Identifier: GPL-3.0-or-later
#include "lock.h"

namespace fms {
namespace platform {

Lock::Lock(Mutex &mutex)
	: m_mutex(mutex) {
	m_mutex.lock();
}

Lock::~Lock() {
	m_mutex.unlock();
}

}
}
