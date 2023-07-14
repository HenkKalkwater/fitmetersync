// SPDX-FileCopyrightText: © 2022 Chris Josten <chris@netsoj.nl>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <ctime>
#include <chrono>
#include <iostream>

#include <pcap.h>
#include <platform/iradapter.h>
#include <protocol/link.h>

#include "command_common.h"

using namespace std::string_literals;

namespace fms {
namespace cli {

/**
 * \brief Generates a filename for the output file based on current date and time
 */
std::string generate_filename() {
	using namespace std::chrono;
	std::string name = "fms-capture-"s;

	auto date = system_clock::to_time_t(system_clock::now());
	auto tm = std::localtime(&date);
	constexpr size_t date_size = std::size("yyyy-mm-ddThh:mm:ss");
	char buf[date_size];
	std::strftime(buf, date_size, "%FT%T", tm);
	name += buf;
	name += ".pcap";
	return name;
}

/**
 * Capture subcommand
 */
int command_capture(common_options &common_options, po::variables_map &specific_options) {
	using namespace platform;
	IRAdapter &adapter = common_options.adapter;
	protocol::LinkConnection con(adapter);

	auto out_opt = specific_options["out-file"];

	std::string filename =  out_opt.empty() ? generate_filename() : out_opt.as<std::string>();

	Pcap pcap(filename);

	std::cerr << "Starting capture, writing to " << filename << std::endl;
	size_t packetCount = 0;
	pcap.writeHeader();

	while (true) {
		auto packet = con.receivePacket();
		std::vector<u8> rawPacket = packet.serializePacket();
		packetCount++;

		PcapRecord record(rawPacket.size(), rawPacket.data());
		pcap.addRecord(record);
		std::cerr << "\rRetrieved " << packetCount << " packets" << std::flush;
	}

	return 0;
}

} // NS cli
} // NS fms
