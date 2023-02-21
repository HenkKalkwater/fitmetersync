// SPDX-License-Identifier: GPL-3.0-or-later
#include "iradaptermanager.h"

namespace fms {
namespace platform {

IRAdapterManager *IRAdapterManager::s_instance = nullptr;

IRAdapterManager &IRAdapterManager::getInstance() {
	if (!s_instance) {
		s_instance = new IRAdapterManager;
	}
	return *s_instance;
}

}
} // NS fms
