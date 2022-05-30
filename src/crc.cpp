/*  
  * crc8.c
 * 
 * Computes a 8-bit CRC 
 * 
 */

#include "crc.h"

//#define GP  0x107   /* x^8 + x^2 + x + 1 */
//#define DI  0x07

namespace fms {
    
	bool made_table = false;
	u8 crc8_table[256];     /* 8-bit table */

	// Should be called before any other crc function.  
	void init_crc8() {
		int i,j;
		unsigned char crc;
		if (!made_table) {
			for (i = 0; i < 256; i++) {
				crc = i;
				for (j = 0; j < 8; j++) {
					crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
				}
				crc8_table[i] = crc & 0xFF;
			}
			made_table = true;
		}
	}
    

	/*
	 * For a byte array whose accumulated crc value is stored in *crc, computes
	 * resultant crc obtained by appending m to the byte array
	 */
	void crc8(u8 *crc,u8 m) {
		if (!made_table)
			init_crc8();
		*crc = crc8_table[(*crc) ^ m];
		*crc &= 0xFF;
	}

	u8 crc8_arr(u8* m, size_t size) {
		u8 crc = 0;
		for (size_t i = 0; i < size; i++) {
			crc8(&crc, m[i]);
		}
		return crc;
	}
}
