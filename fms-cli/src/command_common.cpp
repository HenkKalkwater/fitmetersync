// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "command_common.h"

#include <platform/iradapter.h>
#include <platform/iradaptermanager.h>

#include <iostream>


namespace fms {
namespace cli {

using namespace fms::platform;

std::array<command_description, 3> COMMANDS = {{
	{"capture",       "capture IR communication to a PCAP file",                                  &command_capture},
	{"list-adapters", "lists the available IrDA adapters to use",                                 &command_list_adapters},
	{"retrieve",      "retrieves and parses data from the fit meter (development purposes only)", &command_retrieve}
}};

int command_list_adapters() {
	IRAdapterManager& adapterMgr = IRAdapterManager::getInstance();
	std::cout << "List of adapters:" << std::endl;

	auto adapters = adapterMgr.list();
	for (auto it = adapters.cbegin(); it != adapters.cend(); it++) {
		std::cout << it->name() << std::endl;
	}

	if (adapters.size() == 0) {
		std::cout << "No adapters found";
		return -2;
	}

	return 0;
}


}
}
