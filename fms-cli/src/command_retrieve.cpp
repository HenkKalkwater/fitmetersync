// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <core/types.h>
#include <platform/iradapter.h>
#include <platform/iradaptermanager.h>
#include <protocol/link.h>

#include <iostream>

#include "command_common.h"

namespace fms {
namespace cli {


int command_retrieve() {
	using namespace platform;
	IRAdapterManager& adapterMgr = IRAdapterManager::getInstance();
	IRAdapter adapter = adapterMgr.list()[0];
	fms::protocol::LinkConnection con(adapter);
	con.accept();

	std::vector<u8> data(25);
	//size_t sent = 0;
	//adapter.read(data.begin(), data.end(), sent);
	auto sent = con.receive(data.begin(), data.end());
	std::cout << "Read " << sent << " bytes" << std::endl;
	cli::print_hex(data.begin(), data.begin() + sent);
	return 0;
}

}
}
