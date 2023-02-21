// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef FMS_PLATFORM_LINUX_IRADAPTERMANAGER_H
#define FMS_PLATFORM_LINUX_IRADAPTERMANAGER_H

#include <filesystem>

namespace fms {
namespace platform {

// Handle is a path to a tty node
using IRAdapterHandle = std::filesystem::path;

}
}

#endif // FMS_PLATFORM_CTRU_IRADAPTERMANAGER_H
