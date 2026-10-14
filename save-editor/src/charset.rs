use encoding_rs::SHIFT_JIS;

pub const PE_TERM: u8 = 0xFF;

/// Decodes a PE1 in-game glyph byte slice (terminated by `0xFF`) to a UTF-8 `String`.
/// Matches `pe_code_to_ascii` in `include/pe1/pe_charset.h`.
pub fn pe_decode(bytes: &[u8]) -> String {
    let mut out = String::new();
    for &b in bytes {
        if b == PE_TERM {
            break;
        }
        if let Some(ch) = pe_code_to_char(b) {
            out.push(ch);
        }
    }
    out
}

/// Encodes a UTF-8 string into a fixed-size PE1 glyph buffer, terminated and padded with `0xFF`.
/// Matches `pe_encode` in `include/pe1/pe_charset.h` and `InitName` in `src/main/save/game_data.c`.
pub fn pe_encode_fixed(s: &str, dst: &mut [u8], max_chars: usize) {
    dst.fill(PE_TERM);
    let limit = max_chars.min(dst.len().saturating_sub(1));
    let mut written = 0;
    for ch in s.chars() {
        if written >= limit {
            break;
        }
        if let Some(code) = char_to_pe_code(ch) {
            dst[written] = code;
            written += 1;
        }
    }
    if written < dst.len() {
        dst[written] = PE_TERM;
    }
}

pub fn pe_code_to_char(c: u8) -> Option<char> {
    match c {
        0x00..=0x09 => Some((b'0' + c) as char),
        0x0A => Some('+'),
        0x0B => Some('-'),
        0x0C => Some('='),
        0x0D => Some('*'),
        0x0E => Some('%'),
        0x0F => Some(' '),
        0x10..=0x29 => Some((b'A' + (c - 0x10)) as char),
        0x2A => Some('&'),
        0x2B => Some('!'),
        0x2C => Some('?'),
        0x2D => Some('"'),
        0x2E => Some('\''),
        0x2F => Some(','),
        0x30..=0x49 => Some((b'a' + (c - 0x30)) as char),
        0x4A => Some('.'),
        0x4B => Some(':'),
        0x4C => Some(';'),
        0x4D => Some('/'),
        _ => None,
    }
}

pub fn char_to_pe_code(ch: char) -> Option<u8> {
    match ch {
        '0'..='9' => Some(ch as u8 - b'0'),
        '+' => Some(0x0A),
        '-' => Some(0x0B),
        '=' => Some(0x0C),
        '*' => Some(0x0D),
        '%' => Some(0x0E),
        ' ' => Some(0x0F),
        'A'..='Z' => Some(0x10 + (ch as u8 - b'A')),
        '&' => Some(0x2A),
        '!' => Some(0x2B),
        '?' => Some(0x2C),
        '"' => Some(0x2D),
        '\'' => Some(0x2E),
        ',' => Some(0x2F),
        'a'..='z' => Some(0x30 + (ch as u8 - b'a')),
        '.' => Some(0x4A),
        ':' => Some(0x4B),
        ';' => Some(0x4C),
        '/' => Some(0x4D),
        _ => None,
    }
}

/// Decodes the 64-byte Shift-JIS title in `MemCardFileHeader` (`0x04..0x44`) into a readable string,
/// also converting full-width Shift-JIS ASCII characters to standard half-width ASCII for clean display.
pub fn decode_sjis_title(raw_64: &[u8]) -> (String, String) {
    let nul_pos = raw_64.iter().position(|&b| b == 0).unwrap_or(raw_64.len());
    let (cow, _, _) = SHIFT_JIS.decode(&raw_64[..nul_pos]);
    let fullwidth = cow.into_owned();
    let ascii = fullwidth_to_ascii(&fullwidth);
    (fullwidth, ascii)
}

pub fn fullwidth_to_ascii(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    for ch in s.chars() {
        match ch {
            '\u{3000}' => out.push(' '),
            '\u{2212}' | '\u{FF0D}' => out.push('-'),
            '\u{FF01}'..='\u{FF5E}' => {
                if let Some(a) = char::from_u32(ch as u32 - 0xFEE0) {
                    out.push(a);
                } else {
                    out.push(ch);
                }
            }
            _ => out.push(ch),
        }
    }
    out.trim_end().to_string()
}

/// Formats the 64-byte Shift-JIS save header title exactly like `Save_FormatTitle` / `Save_SprintfSjis`
/// in `src/main/memcard/save_flow.c`.
///
/// - Normal save (`is_clear_data = false`):
///   `ＰＥ−{slot:02}／{HH:02}：{MM:02}：{SS:02}／ＤＡＹ{day}　{location}`
/// - Clear Data / EX Game (`is_clear_data = true`):
///   `ＰＥ−{slot:02}／{HH:02}：{MM:02}：{SS:02}／Ｃｌｅａｒ　Ｄａｔａ`
pub fn format_sjis_title_bytes(
    slot_one_based: usize,
    play_time_seconds: i32,
    day: i16,
    location_label: Option<&str>,
    is_clear_data: bool,
) -> [u8; 64] {
    let secs_total = play_time_seconds.max(0) as u32;
    let hours = (secs_total / 3600) % 100;
    let minutes = (secs_total / 60) % 60;
    let seconds = secs_total % 60;
    let slot_num = slot_one_based.clamp(1, 99);

    let ascii_title = if is_clear_data {
        format!(
            "PE-{:02}/{:02}:{:02}:{:02}/Clear Data",
            slot_num, hours, minutes, seconds
        )
    } else {
        let d = day.clamp(1, 9);
        match location_label {
            Some(loc) if !loc.is_empty() => format!(
                "PE-{:02}/{:02}:{:02}:{:02}/DAY{} {}",
                slot_num, hours, minutes, seconds, d, loc
            ),
            _ => format!(
                "PE-{:02}/{:02}:{:02}:{:02}/DAY{} ",
                slot_num, hours, minutes, seconds, d
            ),
        }
    };

    encode_ascii_as_fullwidth_sjis(&ascii_title)
}

pub fn encode_ascii_as_fullwidth_sjis(ascii: &str) -> [u8; 64] {
    let mut out = [0u8; 64];
    let mut idx = 0;
    for ch in ascii.chars() {
        if idx + 2 >= 64 {
            break;
        }
        let pair: Option<[u8; 2]> = match ch {
            '0'..='9' => Some([0x82, 0x4F + (ch as u8 - b'0')]),
            'A'..='Z' => Some([0x82, 0x60 + (ch as u8 - b'A')]),
            'a'..='z' => Some([0x82, 0x81 + (ch as u8 - b'a')]),
            ' ' => Some([0x81, 0x40]),
            '-' => Some([0x81, 0x7C]),
            '/' => Some([0x81, 0x5E]),
            ':' => Some([0x81, 0x46]),
            '.' => Some([0x81, 0x44]),
            '#' => Some([0x81, 0x94]),
            _ => None,
        };
        if let Some([hi, lo]) = pair {
            out[idx] = hi;
            out[idx + 1] = lo;
            idx += 2;
        }
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pe_charset_roundtrip() {
        let mut buf = [0u8; 13];
        pe_encode_fixed("Aya", &mut buf, 8);
        assert_eq!(&buf[..4], &[0x10, 0x48, 0x30, 0xFF]);
        assert_eq!(pe_decode(&buf), "Aya");
    }

    #[test]
    fn test_sjis_title_matches_retail() {
        let bytes = format_sjis_title_bytes(5, 49551, 5, Some("Museum"), false);
        let expected_hex = "826f8264817c824f8254815e82508252814682538254814682548250815e82638260827882548140826c8295829382858295828d";
        let actual_hex: String = bytes[..52].iter().map(|b| format!("{:02x}", b)).collect();
        assert_eq!(actual_hex, expected_hex);
    }
}
