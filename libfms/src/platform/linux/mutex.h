// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_LINUX_MUTEX_H
#define FMS_PLATFORM_LINUX_MUTEX_H

#include <mutex>

namespace fms {
namespace platform {
/**
 * \brief Alias for the host OS Mutex handle.
 */
using MutexHandle = ::std::mutex;
}
}

#endif // FMS_PLATFORM_CTRU_MUTEX_H
