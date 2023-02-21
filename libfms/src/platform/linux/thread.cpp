// SPDX-License-Identifier: GPL-3.0-or-later

#include "../thread.h"

namespace fms {
namespace platform {

ThreadHandle Thread::createThread(std::function<void()> threadFunction) {
	return std::thread(threadFunction);
}

void Thread::join() {
	return m_handle.join();
}

}
}
