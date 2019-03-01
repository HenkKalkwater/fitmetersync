#ifndef CRC_H
#define CRC_H

#include <3ds.h>

namespace FMS {
    const int GP = 0x107;
    const int DI = 0x7;
    
    extern u8 crc8_table[256];     /* 8-bit table */
    extern bool made_table;

    void init_crc8();
    void crc8(u8 *crc, u8 m);
    u8 crc8_arr(u8* m, size_t size);
}

#endif //CRC_H
