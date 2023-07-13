// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <core/types.h>
#include <platform/iradapter.h>
#include <platform/iradaptermanager.h>
#include <protocol/link.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <utility>

#include <boost/program_options.hpp>

#include "command_common.h"

using namespace fms;
using namespace fms::platform;

namespace po = boost::program_options;

namespace fms {
namespace cli {


/**
 * \brief Prints the usage of this program to stdout.
 */
void print_usage(const char *name, po::options_description desc) {
	std::cout << name << " <command> [options…]" << std::endl;
	std::cout << std::endl;
	std::cout << "Available commands:" << std::endl;

	size_t max_length = std::max_element(COMMANDS.cbegin(), COMMANDS.cend(), [](const command_description &a, const command_description &b) {
		return a.name.size() < b.name.size();
	})->name.size() + 3; // ": "s.size() + 1

	for(auto it = COMMANDS.cbegin(); it != COMMANDS.cend(); it++) {
		std::cout << "  " << std::setw(max_length) << std::left << it->name + ": "
		          << std::setw(0) << it->description << std::endl;
	}
	std::cout << std::endl;
	std::cout << desc << std::endl;
};

} // NS cli
} // NS fms

int main(int argc, char** argv) {
	namespace cli = fms::cli;
	po::options_description desc("General options");
	desc.add_options()
		("help", "produces this help message");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if (argc < 2) {
		cli::print_usage(argc > 0 ? argv[0] : "fms-cli", desc);
		return -1;
	}

	// Print help if requested
	if (vm.count("help")) {
		cli::print_usage(argv[0], desc);
		return -1;
	}

	// Check which command to execute
	for (auto it = cli::COMMANDS.cbegin(); it != cli::COMMANDS.cend(); it++) {
		if (it->name == argv[1]) {
			return it->func();
		}
	}

	std::cout << "No such command '" << argv[1] << "'" << std::endl;
	cli::print_usage(argv[0], desc);
	return -1;
}
