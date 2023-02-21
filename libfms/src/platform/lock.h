// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_LOCK_H
#define FMS_PLATFORM_LOCK_H

#include "mutex.h"

namespace fms {
namespace platform {

class CondVar;

/**
 * \brief RAII wrapper around Mutex
 */
class Lock {
	friend class CondVar;
public:
	/**
	 * \brief Creates a Lock and locks the Mutex.
	 * \param mutex The Mutex to hold and lock.
	 */
	explicit Lock(Mutex &mutex);
	/**
	 * \brief Unlocks the Mutex
	 */
	~Lock();
private:
	Mutex &m_mutex;
};

} // NS platform
} // NS fms

#endif // FMS_PLATFORM_LOCK_H
