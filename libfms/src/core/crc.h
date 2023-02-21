#ifndef CRC_H
#define CRC_H

#include <core/types.h>

#include <cstddef>

namespace fms {
	const int GP = 0x107;
	const int DI = 0x7;

	extern u8 crc8_table[256];     /* 8-bit table */
	extern bool made_table;

	void init_crc8();
	void crc8(u8 &crc, u8 m);
	u8 crc8_arr(const u8 *m, std::size_t size, u8 start = 0);
}

#endif //CRC_H
