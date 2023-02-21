// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_CONDVAR_H
#define FMS_PLATFORM_CONDVAR_H

#ifdef __3DS__
#include "ctru/condvar.h"
#endif // __3DS__

#ifdef __linux__
#include "linux/condvar.h"
#endif // __linux__

#include <platform/lock.h>
#include <core/types.h>

namespace fms {
namespace platform {


/**
 * \brief Condition variable
 *
 * Not shared among processes
 */
class CondVar {
public:
	CondVar();
	~CondVar();

	/**
	 * \brief Wakes up a single thread waiting on this condition variable
	 */
	void wakeup();
	/**
	 * \brief Wakes up all threads waiting on this condition variable.
	 */
	void wakeupAll();

	/**
	 * \brief Waits on this condition variable.
	 */
	void wait(Lock &lock);
	/**
	 * \brief Waits on the condition variable with a timeout
	 * \param[in] lock THe already locked lock to hold while waiting.
	 * \param[in] duration The time to wait before timing out.
	 * \return True when the wait did not time out.
	 */
	bool waitFor(Lock &lock, time_ns duration);
private:
	CondVarHandle m_handle;
};

} // NS platform
} // NS fms

#endif // FMS_PLATFORM_CONDVAR_H
