// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_CTRU_IRADAPTERMANAGER_H
#define FMS_PLATFORM_CTRU_IRADAPTERMANAGER_H

#include <cstddef>

namespace fms {
namespace platform {

// There is only 1 IR adapter available on the 3DS,
// the IRAdapterDescriptor is superfluous on this platform.
using IRAdapterHandle = std::nullptr_t;

}
}

#endif // FMS_PLATFORM_CTRU_IRADAPTERMANAGER_H
