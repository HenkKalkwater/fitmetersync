// SPDX-License-Identifier: GPL-3.0-or-later
#include "../iradaptermanager.h"

namespace fms {
namespace platform {

IRAdapterDescriptor::IRAdapterDescriptor(IRAdapterHandle &&handle)
	: m_handle(handle) {}


IRAdapterDescriptor::~IRAdapterDescriptor() {}

std::string IRAdapterDescriptor::name() const {
	return "Default IR adapter";
}

class IRAdapterManagerPrivate {};
IRAdapterManager::IRAdapterManager() {}
IRAdapterManager::~IRAdapterManager() {}



std::vector<IRAdapterDescriptor> IRAdapterManager::list() {
	// There is only one IRAdapter on the Nintendo 3DS.
	static std::vector<IRAdapterDescriptor> IR_ADAPTER_LIST = { IRAdapterDescriptor(nullptr) };
	return IR_ADAPTER_LIST;
}

}
} // NS fms
