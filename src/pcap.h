#ifndef PCAP_H
#define PCAP_H

#include "3ds.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>

namespace FMS {
    /**
    * Single record in the Pcap file
    * \see https://wiki.wireshark.org/Development/LibpcapFileFormat
    */
    struct PcapRecord {
		PcapRecord(u32 length, void * data) {
			// Time in millis
			u64 currTime = osGetTime();
			this->ts_sec = currTime / 1000;
			// ts_usec has to be in microseconds
			this->ts_usec = currTime % 1000 * 1000;
			
            this->orig_length = length;
            this->data = std::malloc(length);
            if (this->data != nullptr) std::memcpy(this->data, data, length);
		}
        PcapRecord(u32 ts_sec, u32 ts_usec, u32 length, void* data) {
            this->ts_sec = ts_sec;
            this->ts_usec = ts_usec;
            this->orig_length = length;
            this->data = std::malloc(length);
            if (this->data != nullptr) std::memcpy(this->data, data, length);
        }
        u32 ts_sec;
        u32 ts_usec;
        u32 orig_length;
        void* data;
    };

    /**
    * Represents a capture session. 
    * 
    * \see https://wiki.wireshark.org/Development/LibpcapFileFormat
    */
    class Pcap {
    public:
		Pcap(std::string filename) : ofile(filename, std::ios::binary) {}
        /**
        * Adds a record to the list.
        */
        void addRecord(PcapRecord record);
        /**
        * Write everything that's caputered to the file.
        */
        void flush();

		bool isOpen() { return ofile.is_open(); }
		u32 getSnaplen() { return snaplen; }
    
    private:
        /**
        * Magic number of the PCAP file format
        */
        const u32 magic = 0xa1b2c3d4;
        
        /**
        * Major version of the PCAP file format
        */
        const u16 versionMajor = 2;
        
        /**
        * Minor version of the PCAP file format
        */
        const u16 versionMinor = 4;
        
        /**
        * Offset from GMT time.
        */
        const s32 thiszone = 0;
        
        /**
        * Accuracy of the timestamps.
        */
        const u32 sigfigs = 0;
        
        /**
        * Maximum length of recorded packets
        */
        const u32 snaplen = 65535;
        
        /**
        * Network type
        * \see http://www.tcpdump.org/linktypes.html
        */
        const u32 network = 147; // RESERVED FOR PRIVATE USE
        
        std::vector<PcapRecord> records;
		std::ofstream ofile;
    };
}

#endif //PCAP_H
