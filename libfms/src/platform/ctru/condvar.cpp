// SPDX-License-Identifier: GPL-3.0-or-later
#include "../condvar.h"

#include <3ds.h>

namespace fms {
namespace platform {

CondVar::CondVar()
	: m_handle(new ::CondVar()){
	::CondVar_Init(m_handle);
}

CondVar::~CondVar() {
	delete m_handle;
}

void CondVar::wakeup() {
	::CondVar_Signal(m_handle);
}

void CondVar::wakeupAll() {
	::CondVar_WakeUp(m_handle, ARBITRATION_SIGNAL_ALL);
}
void CondVar::wait(Lock &lock) {
	::CondVar_Wait(m_handle, lock.m_mutex.m_lock);
};
bool CondVar::waitFor(Lock &lock, time_ns duration) {
	return !static_cast<bool>(::CondVar_WaitTimeout(m_handle, lock.m_mutex.m_lock, duration));
}

}
}
