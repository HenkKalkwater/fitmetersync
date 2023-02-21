// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_LINUX_CONDVAR_H
#define FMS_PLATFORM_LINUX_CONDVAR_H

#include <condition_variable>

namespace fms {
namespace platform {

using CondVarHandle = ::std::condition_variable;

} // NS platform
} // NS fms

#endif // FMS_PLATFORM_LINUX_CONDVAR_H
