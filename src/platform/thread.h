#ifndef FMS_PLATFORM_THREAD_H
#define FMS_PLATFORM_THREAD_H

#ifdef __3DS__
#include "ctru/thread.h"
#endif // __3DS__

#include <functional>
#include <memory>
#include <tuple>

namespace fms {
namespace platform {

/**
 * \brief OS Thread wrapper
 */
class Thread {
public:
	template <class Function, class... Args>
	explicit Thread(Function&& function, Args&&... args) {
		auto fn = std::bind(std::forward<Function>(function), std::forward<Args>(args)...);
		m_handle = createThread([fn]() { (*fn)(); });
	}
private:
	ThreadHandle createThread(std::function<void()> threadFunction);
	static void threadProc(void *args);

	ThreadHandle m_handle;
};

}
}

#endif // FMS_PLATFORM_THREAD_H
