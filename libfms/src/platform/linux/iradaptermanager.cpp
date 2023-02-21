// SPDX-License-Identifier: GPL-3.0-or-later
#include "../iradaptermanager.h"

namespace fms {
namespace platform {

IRAdapterDescriptor::IRAdapterDescriptor(IRAdapterHandle &&handle)
	: m_handle(handle) {}


IRAdapterDescriptor::~IRAdapterDescriptor() {}

std::string IRAdapterDescriptor::name() const {
	return m_handle.filename().string();
}

class IRAdapterManagerPrivate {};
IRAdapterManager::IRAdapterManager() {}
IRAdapterManager::~IRAdapterManager() {}

std::vector<IRAdapterDescriptor> IRAdapterManager::list() {
	// TODO: implement
	static std::vector<IRAdapterDescriptor> IR_ADAPTER_LIST = {
		IRAdapterDescriptor(std::filesystem::path("/dev/ttyUSB0"))
	};
	return IR_ADAPTER_LIST;
}

}
} // NS fms

