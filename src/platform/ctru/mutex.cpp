#include "../mutex.h"

#include <3ds.h>

namespace fms {
namespace platform {

Mutex::Mutex()
	:m_lock(new ::LightLock() ) {
	::LightLock_Init(m_lock);
}

Mutex::Mutex(Mutex &&other)
	: m_lock(other.m_lock) {
	other.m_lock = nullptr;
}

Mutex::~Mutex() {
	if (m_lock) {
		delete m_lock;
	}
}

void Mutex::lock() {
	::LightLock_Lock(m_lock);
}

void Mutex::unlock() {
	::LightLock_Unlock(m_lock);
}

bool Mutex::try_lock() {
	return !static_cast<bool>(::LightLock_TryLock(m_lock));
}


} // NS platform
} // NS fms
