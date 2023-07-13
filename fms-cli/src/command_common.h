// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FMS_CLI_COMMAND_COMMON_H
#define FMS_CLI_COMMAND_COMMON_H

#include <boost/program_options.hpp>

#include <iomanip>
#include <iostream>
#include <string>

namespace fms {
namespace cli {

struct command_description {
	/// Name of the command
	std::string name;
	/// Help description for the command
	std::string description;
	/// The function to call when this command is invoked
	std::function<int()> func;
};

struct common_options {

};


/**
 * Prints the data from the iterator formatted as hex to standard out.
 */
template<typename It>
void print_hex(It begin, It end) {
	int i = 1;
	for (auto it = begin; it != end; it++) {
		std::cout << std::hex <<  std::setw(2) << std::setfill('0') << unsigned(*it);
		if (i % 4 == 0) std::cout << " ";
		if (i % 16 == 0) std::cout << std::endl;
		i++;
	}
	std::cout << std::endl;
}

// list of commands
int command_capture();
int command_list_adapters();
int command_retrieve();

extern std::array<command_description, 3> COMMANDS;

} // NS cli
} // NS fms

#endif // FMS_CLI_COMMAND_COMMON_H
