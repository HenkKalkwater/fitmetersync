// SPDX-License-Identifier: GPL-3.0-or-later
#include "pcap.h"

#include <chrono>

namespace fms {
	PcapRecord::PcapRecord(u32 length, void * data){
		// Time in millis
		//u64 currTime = osGetTime();
		u64 currTime = std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
		this->ts_sec = currTime / 1000'000;
		// ts_usec has to be in microseconds
		this->ts_usec = currTime % 1000'000;

		this->orig_length = length;
		this->data = std::malloc(length);
		if (this->data != nullptr) std::memcpy(this->data, data, length);
	}
	PcapRecord::PcapRecord(u32 ts_sec, u32 ts_usec, u32 length, void* data) {
		this->ts_sec = ts_sec;
		this->ts_usec = ts_usec;
		this->orig_length = length;
		this->data = std::malloc(length);
		if (this->data != nullptr) std::memcpy(this->data, data, length);
	}

	Pcap::~Pcap() {
		ofile.close();
	}

	void Pcap::writeHeader() {
		//Write the header
		ofile.write((char *)&this->magic, sizeof(u32));
		ofile.write((char *)&this->versionMajor, sizeof(u16));
		ofile.write((char *)&this->versionMinor, sizeof(u16));
		ofile.write((char *)&this->thiszone, sizeof(s32));
		ofile.write((char *)&this->sigfigs, sizeof(u32));
		ofile.write((char *)&this->snaplen, sizeof(u32));
		ofile.write((char *)&this->network, sizeof(u32));
		ofile.flush();
	}

	void Pcap::addRecord(PcapRecord record) {
		ofile.write((char *) &record.ts_sec, sizeof(u32));
		ofile.write((char *) &record.ts_usec, sizeof(u32));
		ofile.write((char *) &record.orig_length, sizeof(u32));

		u32 incl_length = std::min(record.orig_length, this->snaplen);
		ofile.write((char *) &incl_length, sizeof(u32));
		ofile.write((char *) record.data, incl_length);
		ofile.flush();
	}
}
