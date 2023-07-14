// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <core/debug.h>
#include <core/types.h>
#include <platform/iradapter.h>
#include <platform/iradaptermanager.h>
#include <protocol/link.h>

#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/program_options.hpp>

#include "command_common.h"

using namespace fms;
using namespace fms::platform;
using namespace std::string_literals;

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

IRAdapter find_adapter(std::string name) {
	IRAdapterManager &manager = IRAdapterManager::getInstance();
	auto adapters = manager.list();
	if (adapters.empty()) {
		throw new std::runtime_error("No available IrDA adapters");
	}
	if (name.empty()) {
		return adapters.at(0);
	}

	for (auto it = adapters.begin(); it != adapters.end(); it++) {
		if (it->name() == name) {
			return *it;
		}
	}
	throw new std::runtime_error("No adapter with the given name '"s + name + "'"s);
}

} // NS cli
} // NS fms

int main(int argc, char** argv) {
	namespace cli = fms::cli;
	po::options_description desc("General options");
	desc.add_options()
		("help", "produces this help message")
		("irda-adapter", po::value<std::string>(),  "name of an irda adapter, defaults to first found, see 'list-adapters'");

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

	try {
		auto adapter_opt = vm["irda-adapter"s];
		IRAdapter adapter = cli::find_adapter(adapter_opt.empty() ? "" : adapter_opt.as<std::string>());

		cli::common_options options = {
			adapter
		};

		// Check which command to execute
		for (auto it = cli::COMMANDS.cbegin(); it != cli::COMMANDS.cend(); it++) {
			if (it->name == argv[1]) {
				return it->func(options);
			}
		}

		// Print usage and exit if command is not known
		std::cout << "No such command '" << argv[1] << "'" << std::endl;
		cli::print_usage(argv[0], desc);
		return -1;
	} catch(std::exception *e) {
		FMS_ERR("Uncaught exception: %s", e->what());
	}
}
