// SPDX-License-Identifier: GPL-3.0-or-later
#include "../thread.h"

#include <3ds.h>

namespace fms {
namespace platform {
/// Thread entry point
static void threadProc(void *args);

ThreadHandle Thread::createThread(std::function<void()> threadFunc) {
	::s32 prio;
	::svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

	m_handle = ::threadCreate(&threadProc, &threadFunc, /*stackSize=*/ 0x200, prio, /*coreId=*/ -2, false);
	return m_handle;
}

void threadProc(void *args) {
	auto fn = static_cast<std::function<void()> *>(args);
	(*fn)();
}

void Thread::join() {
	::threadJoin(m_handle, U64_MAX);
}

} // NS platform
} // NS fms
