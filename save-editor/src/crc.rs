/// Offset of the 4-byte CRC-16 word inside a 0x2000-byte Parasite Eve save block.
///
/// Layout:
/// - `0x0000..0x0100`: `MemCardFileHeader` (`0x100` bytes)
/// - `0x0100..0x13E4`: `SaveRuntimeState` (`0x12E4` bytes)
/// - `0x13E4..0x1C8C`: `Save_SerializeTail` (`0x8A8` bytes)
/// - `0x1C8C..0x1C90`: `u32` CRC16 checksum (`~crc & 0xFFFF`)
pub const SAVE_BLOCK_SIZE: usize = 0x2000;
pub const SAVE_CRC_OFFSET: usize = 0x1C8C;

/// Calculates the Parasite Eve save block CRC-16 (`Save_CalcCrc16` in `src/main/memcard/save_flow.c`).
///
/// In retail and the PC port, the CRC is computed over all `0x2000` bytes of the save file
/// buffer while the 4 bytes at `0x1C8C..0x1C90` are zeroed out.
pub fn calc_save_crc16(block: &[u8]) -> u16 {
    assert!(
        block.len() >= SAVE_BLOCK_SIZE,
        "Save block must be at least 0x2000 bytes"
    );
    let mut crc: u32 = 0xFFFF;
    for (i, &b) in block[..SAVE_BLOCK_SIZE].iter().enumerate() {
        let byte = if (SAVE_CRC_OFFSET..SAVE_CRC_OFFSET + 4).contains(&i) {
            0u8
        } else {
            b
        };
        crc ^= (byte as u32) << 8;
        for _ in 0..8 {
            if (crc & 0x8000) != 0 {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    (!crc & 0xFFFF) as u16
}

/// Reads the stored 32-bit CRC field at `0x1C8C` from a `0x2000`-byte save block.
pub fn read_stored_crc(block: &[u8]) -> u32 {
    u32::from_le_bytes([
        block[SAVE_CRC_OFFSET],
        block[SAVE_CRC_OFFSET + 1],
        block[SAVE_CRC_OFFSET + 2],
        block[SAVE_CRC_OFFSET + 3],
    ])
}

/// Computes and writes the updated CRC-16 into `0x1C8C..0x1C90` of a `0x2000`-byte save block.
pub fn update_save_crc16(block: &mut [u8]) -> u16 {
    block[SAVE_CRC_OFFSET..SAVE_CRC_OFFSET + 4].fill(0);
    let crc = calc_save_crc16(block);
    let crc_bytes = (crc as u32).to_le_bytes();
    block[SAVE_CRC_OFFSET..SAVE_CRC_OFFSET + 4].copy_from_slice(&crc_bytes);
    crc
}

/// Computes the XOR checksum byte (`frame[0x7F]`) of a 128-byte PS1 Memory Card directory frame,
/// matching `_card_sector_write` in PsyZ (`third_party/psyz/psyz/src/psyz/libcard.c`).
pub fn calc_mc_frame_xor(frame: &[u8]) -> u8 {
    assert!(frame.len() >= 0x80);
    let mut xor = 0u8;
    for &b in &frame[..0x7F] {
        xor ^= b;
    }
    xor
}

/// Updates the final byte (`0x7F`) of a 128-byte PS1 Memory Card frame with its XOR checksum.
pub fn update_mc_frame_xor(frame: &mut [u8]) {
    let xor = calc_mc_frame_xor(frame);
    frame[0x7F] = xor;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_crc_roundtrip() {
        let mut block = vec![0u8; SAVE_BLOCK_SIZE];
        block[0] = b'S';
        block[1] = b'C';
        block[0x100] = 0x10;
        let crc = update_save_crc16(&mut block);
        assert_eq!(read_stored_crc(&block), crc as u32);
        assert_eq!(calc_save_crc16(&block), crc);
    }
}
