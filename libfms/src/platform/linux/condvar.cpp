// SPDX-License-Identifier: GPL-3.0-or-later
#include "../condvar.h"

namespace fms {
namespace platform {
CondVar::CondVar() : m_handle(std::condition_variable()) {}

CondVar::~CondVar() {}

void CondVar::wakeup() {
	m_handle.notify_one();
}

void CondVar::wakeupAll() {
	m_handle.notify_all();
}

void CondVar::wait(Lock &lock) {
	std::unique_lock stdLock(lock.m_mutex.m_lock, std::adopt_lock);
	m_handle.wait(stdLock);
}

bool CondVar::waitFor(Lock &lock, time_ns duration) {
	std::unique_lock stdLock(lock.m_mutex.m_lock, std::adopt_lock);
	return m_handle.wait_for(stdLock, std::chrono::nanoseconds(duration)) == std::cv_status::no_timeout;
}

}
}
