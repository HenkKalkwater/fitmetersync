use std::iter::repeat;
use crate::date::DateTime;

enum DecompressedData {
    None,
    Value(u8),
    DateTime(DateTime)
}

pub fn decompress_u8(data: &[u8]) -> Vec<Option<u8>> {
    let mut result = Vec::with_capacity(data.len());
    let mut i = 0;
    let mut last_value = None;

    while i < data.len() {
        let value = data[i];
        match value {
            0xFC => {
                // There is supposed to be a date

            },
            0xFD => return result,
            0xFE => {
                // RLE: read next byte n and repeat last value n times
                i += 1;
                if i >= data.len() {
                    return result;
                }
                let repeat_count = data[i];
                for _ in 0..repeat_count {
                    result.push(last_value);
                }
            }
            0xFF => {
                last_value = None;
                result.push(None)
            },
            _ => {
                last_value = Some(value);
                result.push(Some(value))
            },
        }
        i += 1
    }
    result
}

pub fn decompress_altitude(data: &[u8]) -> Vec<i32> {
    let mut result = Vec::with_capacity(data.len());
    let mut current = 0;
    let mut offset = 0;

    let mut i = 0;
    while i < data.len() {
        match data[i] {
            0xFA => {
                offset += 127;
                i += 1;
            },
            0xFB => {
                offset -= 120;
                i += 1;
            },
            0xFC => {
                // ???
                i += 1;
            },
            0xFD => {
                // ???
                i += 1;
            },
            0xFE => {
                break
            },
            0xFF => {
                // ???
                i += 1;
            },
            v if i + 1 < data.len() && data[i + 1] == 0 => {
                result.extend(repeat(current).take(v.into()));
                i += 2;
            },
            v => {
                let delta = if v <= 0x7F { v as i32 } else { 0x80 - (v as i32) - 1 };
                current = current + offset + delta;
                offset = 0;
                i += 1;
            }
        }
    }

    result
}

pub fn decompress_u16(data: &[u8]) -> Vec<Option<u16>> {
    let mut result = Vec::with_capacity(data.len());
    let mut i = 0;
    let mut last_value = None;

    while i + 1 < data.len() {
        let value = u16::from_be_bytes(data[i..i+2].try_into().unwrap());
        match value {
            0xFFFC => {
                // RLE: read next byte n and repeat last value n times
                i += 2;
                if i + 1 >= data.len() {
                    return result;
                }
                let repeat_count = u16::from_be_bytes(data[i..i+2].try_into().unwrap());
                for _ in 0..repeat_count {
                    result.push(last_value);
                }
            },
            0xFFFD => {
                // There is supposed to be a date
                let day = u16::from_be_bytes(data[i+2..i+4].try_into().unwrap());
                let unk = u16::from_be_bytes(data[i+4..i+6].try_into().unwrap());
                let month = u16::from_be_bytes(data[i+6..i+8].try_into().unwrap());
                let year = u16::from_be_bytes(data[i+8..i+10].try_into().unwrap());
                i += 10;
            },
            0xFFFE => {
                return result
            }
            0xFFFF => {
                last_value = None;
                result.push(None)
            },
            _ => {
                last_value = Some(value);
                result.push(Some(value))
            },
        }
        i += 2
    }
    result
}

#[cfg(test)]
mod tests {
    use crate::compression::decompress_u16;

    #[test]
    fn test_decompress_steps() {
        let compressed_data = [1u8, 135, 1, 186, 2, 130, 1, 132, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 5, 0, 8, 0, 4, 0, 9, 0, 26, 255, 253, 255, 254];
        let data = decompress_u16(&compressed_data);
        assert_eq!(data, [Some(1u16)]);
    }

}