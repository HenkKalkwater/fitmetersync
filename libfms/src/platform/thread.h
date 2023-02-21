// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_THREAD_H
#define FMS_PLATFORM_THREAD_H

#ifdef __3DS__
#include "ctru/thread.h"
#endif // __3DS__

#ifdef __linux__
#include "linux/thread.h"
#endif // __linux__

#include <functional>
#include <future>
#include <memory>
#include <tuple>

namespace fms {
namespace platform {

/**
 * \brief OS Thread wrapper
 */
class Thread {
public:
	/**
	 * \brief Creates and starts a thread
	 * \param function The function to start the thread in
	 * \param args The arguments passed to function
	 *
	 * The arguments will be moved to the new thread and therefore should not
	 * be used in this thread.
	 */
	template <class Function, class... Args>
	explicit Thread(Function&& function, Args&&... args) {
		auto bindFn = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
		m_handle = createThread( [fn = std::move(bindFn)]() { std::invoke(fn); }) ;
	}
	/**
	 * \brief Waits until the thread is done executing
	 */
	void join();

private:
	ThreadHandle createThread(std::function<void()> threadFunction);

	ThreadHandle m_handle;
};

}
}

#endif // FMS_PLATFORM_THREAD_H
