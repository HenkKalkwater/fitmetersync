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
namespace platform {
// Forward declarations
class IRAdapter;
}
namespace cli {

/**
 * Options that will be passed to all classes
 */
struct common_options {
	platform::IRAdapter &adapter;
};

struct command_description {
	/// Name of the command
	std::string name;
	/// Help description for the command
	std::string description;
	/**
	 * \brief The function to call when this command is invoked
	 * \param common_options The common command line options
	 * \returns An exit code
	 */
	std::function<int(common_options &common_options)> func;
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
int command_capture(common_options &common_options);
int command_list_adapters(common_options &common_options);
int command_retrieve(common_options &common_options);

extern std::array<command_description, 3> COMMANDS;

} // NS cli
} // NS fms

#endif // FMS_CLI_COMMAND_COMMON_H
