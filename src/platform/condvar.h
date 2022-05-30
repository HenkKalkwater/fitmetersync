#ifndef FMS_PLATFORM_CONDVAR_H
#define FMS_PLATFORM_CONDVAR_H

#ifdef __3DS__
#include "ctru/condvar.h"
#endif // __3DS__

#include "lock.h"

namespace fms {
namespace platform {
using time_ns = unsigned long long;

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
	 * \return True when the wait did not time out.
	 */
	bool waitFor(Lock &lock, time_ns duration);
private:
	CondVarHandle m_handle;
};

} // NS platform
} // NS fms

#endif // FMS_PLATFORM_CONDVAR_H
