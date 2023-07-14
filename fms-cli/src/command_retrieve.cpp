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
namespace po = boost::program_options;

int command_retrieve(common_options &common_options, po::variables_map &/*specific_options*/) {
	using namespace platform;
	IRAdapter &adapter = common_options.adapter;
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
