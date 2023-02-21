// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_MUTEX_H
#define FMS_PLATFORM_MUTEX_H

#ifdef __3DS__
#include "ctru/mutex.h"
#endif // __3DS__

#ifdef __linux__
#include "linux/mutex.h"
#endif // __linux__

namespace fms {
namespace platform {

class CondVar;

/**
 *
 * \brief %Mutex abstraction.
 *
 * The implementation may be an user-space mutex,
 * don't use it accross process boundaries.
 */
class Mutex {
	friend class CondVar;
public:
	/**
	 * \brief Creates a new unlocked mutex.
	 */
	Mutex();
	Mutex(Mutex &other) = delete;
	Mutex(Mutex &&other) = delete;
	~Mutex();

	/**
	 * \brief Locks the mutex. May block.
	 * \see
	 * tryLock() for a non-blocking version
	 * unlock() for unlocking the mutex.
	 */
	void lock();
	/**
	 * \brief Unlocks the mutex.
	 */
	void unlock();
	/**
	 * \brief Tries locking the mutex. Does not block.
	 * \return True if the mutex was sucessfully locked,
	 * false if the mutex couldn't be locked.
	 */
	bool try_lock();

private:
	MutexHandle m_lock;
};

}
}

#endif // FMS_PLATFORM_MUTEX_H
