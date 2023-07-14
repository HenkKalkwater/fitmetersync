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
#include <optional>
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
void print_usage(const char *name, po::options_description desc, std::optional<command_description> command = std::nullopt) {
	if (command.has_value()) {
		std::cout << name << " " << command->name << " [options…]" << std::endl;
		std::cout << std::endl;
		std::cout << command->description << std::endl;
	} else {
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
	}
	std::cout << desc << std::endl;
};

std::optional<command_description> find_command(std::string name) {
	for (auto it = cli::COMMANDS.cbegin(); it != cli::COMMANDS.cend(); it++) {
		if (it->name == name) {
			return std::make_optional<command_description>(*it);
		}
	}
	return std::nullopt;
}

/**
 * Finds the user-specified IrDA adapter by name
 * \param name Name of the adapter. Empty string picks the first adapter found
 */
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
	po::options_description hidden_desc("Hidden options");
	hidden_desc.add_options()
		("command", "the command to execute")
		("unused-args", po::value<std::vector<std::string>>(), "unused arguments");

	po::options_description desc("General options");
	desc.add_options()
		("help,h", po::value<std::string>()->value_name("command")->implicit_value("all"), "produces a help message for a given command or all commands")
		("irda-adapter,a", po::value<std::string>(),  "name of an irda adapter, defaults to first found, see 'list-adapters'");

	po::options_description parser_desc;
	parser_desc.add(hidden_desc);
	parser_desc.add(desc);

	if (argc < 2) {
		cli::print_usage(argc > 0 ? argv[0] : "fms-cli", desc);
		return -1;
	}

	po::positional_options_description pos_desc;
	pos_desc.add("command", 1).add("unused-args", -1);

	po::command_line_parser parser(argc, argv);
	parser.options(parser_desc);
	parser.positional(pos_desc);
	parser.allow_unregistered();

	po::variables_map vm;
	// First run is for detecting --help, --version and --command
	po::parsed_options parsed_options = parser.run();
	po::store(parsed_options, vm);
	po::notify(vm);

	// Print help if requested
	if (vm.count("help")) {
		auto help_opt = vm["help"];

		if (help_opt.defaulted() || help_opt.as<std::string>() == "all") {
			cli::print_usage(argv[0], desc);
		} else {
			// Print command help as well
			auto cmd = cli::find_command(help_opt.as<std::string>());
			if (cmd.has_value()) {
				po::options_description combined_options;
				combined_options.add(cmd->options);
				combined_options.add(desc);
				cli::print_usage(argv[0], combined_options, cmd);
			} else {
				std::cout << "No such command '" << help_opt.as<std::string>() << "'" << std::endl;
				cli::print_usage(argv[0], desc);
			}
		}
		return -1;
	}

	try {
		auto command_opt = vm["command"];
		std::optional<cli::command_description> command = command_opt.empty()
			? std::nullopt
			: cli::find_command(command_opt.as<std::string>());
		std::cout << "Command count: " << vm.count("command") << std::endl;

		std::cout << command_opt.as<std::string>() << std::endl;

		// Check which command to execute
		if (command.has_value()) {
			auto adapter_opt = vm["irda-adapter"s];
			IRAdapter adapter = cli::find_adapter(adapter_opt.empty() ? "" : adapter_opt.as<std::string>());

			cli::common_options options = {
				adapter
			};

			try {
			auto remaining_options = po::collect_unrecognized(parsed_options.options, po::include_positional);
			po::command_line_parser command_parser(remaining_options);
			command_parser.allow_unregistered();
			po::variables_map command_vm;
			command_parser.options(command->options);
			po::store(command_parser.run(), command_vm);
			po::notify(command_vm);
			command->func(options, command_vm);
			} catch(const po::too_many_positional_options_error &e) {}

		} else {
			// Print usage and exit if command is not known
			std::cout << "No such command '" << command_opt.as<std::string>() << "'" << std::endl;
			cli::print_usage(argv[0], desc);
		}
		return -1;
	} catch(std::exception *e) {
		FMS_ERR("Uncaught exception: %s", e->what());
	}
}
