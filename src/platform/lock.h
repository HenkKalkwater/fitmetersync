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
	explicit Lock(Mutex &mutex);
	~Lock();
private:
	Mutex &m_mutex;
};

} // NS platform
} // NS fms

#endif // FMS_PLATFORM_LOCK_H
