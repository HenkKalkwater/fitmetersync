use std::io::IoSlice;
use std::ops::Deref;

/// Generate a CRC lookup table
const fn generate_crc_table(polynomial: u8) -> [u8; 256] {
    let mut crc_table: [u8; 256] = [0; 256];

    let mut i = 0;
    while i < crc_table.len() {
        let mut crc: u8 = i as u8;

        let mut j = 0;
        while j < 8 {
            crc = (crc << 1) ^ if crc & 0x80 > 0 { polynomial } else { 0 };
            j += 1;
        }

        crc_table[i] = crc;
        i += 1;
    }

    crc_table
}

const IRC_POLYNOMIAL: [u8; 256] = generate_crc_table(0x7);

fn crc_calculate_continue(lookup_table: &[u8; 256], data: &[u8], init: u8) -> u8 {
    let mut crc: u8 = init;
    for byte in data {
        crc = lookup_table[(crc ^ byte) as usize];
    }
    crc
}
fn crc_calculate(lookup_table: &[u8; 256], data: &[u8]) -> u8 {
    crc_calculate_continue(lookup_table, data, 0)
}

fn crc_calculate_continue_ioslice(lookup_table: &[u8; 256], bufs: &[IoSlice], init: u8) -> u8 {
    let mut crc: u8 = init;
    for buf in bufs {
        for byte in buf.deref() {
            crc = lookup_table[(crc ^ byte) as usize];
        }
    }
    crc
}
fn crc_calculate_ioslice(lookup_table: &[u8; 256], bufs: &[IoSlice]) -> u8 {
    crc_calculate_continue_ioslice(lookup_table, bufs, 0)
}

pub fn irc_crc(data: &[u8]) -> u8 {
    crc_calculate(&IRC_POLYNOMIAL, data)
}

pub fn irc_crc_continue(data: &[u8], init: u8) -> u8 {
    crc_calculate_continue(&IRC_POLYNOMIAL, data, init)
}

pub fn irc_crc_ioslice(bufs: &[IoSlice]) -> u8 {
    crc_calculate_ioslice(&IRC_POLYNOMIAL, bufs)
}

pub fn irc_crc_ioslice_continue(bufs: &[IoSlice], init: u8) -> u8 {
    crc_calculate_continue_ioslice(&IRC_POLYNOMIAL, bufs, init)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_crc() {
        let test_packet = vec![0xa5, 0x00, 0x84, 0x01, 0x03, 0x04, 0xeb];
        assert_eq!(irc_crc(&test_packet), 0xf9)
    }
}