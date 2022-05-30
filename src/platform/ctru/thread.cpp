#include "../thread.h"

#include <3ds.h>

namespace fms {
namespace platform {

ThreadHandle Thread::createThread(std::function<void()> threadFunc) {
	::s32 prio;
	::svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);

	m_handle = ::threadCreate(&Thread::threadProc, &threadFunc, /*stackSize=*/ 0x200, prio, /*coreId=*/ -2, false);
}

void Thread::threadProc(void *args) {
	auto fn = static_cast<std::function<void()> *>(args);
	(*fn)();
}

} // NS platform
} // NS fms
