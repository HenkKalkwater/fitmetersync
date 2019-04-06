#include "pcap.h"

namespace FMS {
	void Pcap::addRecord(PcapRecord record) {
		this->records.push_back(record);
	}

	void Pcap::flush() {
		//std::ofstream ofile("foo.pcap", std::ios::binary);
		
		//Write the header
		ofile.write((char *)&this->magic, sizeof(u32));
		ofile.write((char *)&this->versionMajor, sizeof(u16));
		ofile.write((char *)&this->versionMinor, sizeof(u16));
		ofile.write((char *)&this->thiszone, sizeof(s32));
		ofile.write((char *)&this->sigfigs, sizeof(u32));
		ofile.write((char *)&this->snaplen, sizeof(u32));
		ofile.write((char *)&this->network, sizeof(u32));

		for (auto it = this->records.begin(); it < records.end(); it++) {
			ofile.write((char *) &it->ts_sec, sizeof(u32));
			ofile.write((char *) &it->ts_usec, sizeof(u32));
			ofile.write((char *) &it->orig_length, sizeof(u32));

			u32 incl_length = std::min(it->orig_length, this->snaplen);
			ofile.write((char *) &incl_length, sizeof(u32));
			ofile.write((char *) it->data, incl_length);
		}

		ofile.close();
	} 
}
