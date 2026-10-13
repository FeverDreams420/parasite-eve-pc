import os
import struct
import unittest

from tools.scripts import shift_disc as sd


def bitwise_edc(data: bytes) -> int:
    """The CD-ROM EDC, bit by bit: CRC-32 with the reflected polynomial
    0xD8018001 (x^32 + x^31 + x^16 + x^15 + x^4 + x^3 + x + 1), no inversion."""
    value = 0
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0xD8018001 if value & 1 else 0)
    return value


def form1_sector(lba: int, user: bytes) -> bytes:
    """A raw Mode 2 Form 1 sector: sync, BCD address, mode 2, subheader."""
    def bcd(n: int) -> int:
        return (n // 10) << 4 | n % 10
    lba += 150
    header = bytes([bcd(lba // 4500), bcd(lba // 75 % 60), bcd(lba % 75), 2])
    raw = b"\x00" + b"\xFF" * 10 + b"\x00" + header + b"\x00\x00\x08\x00" * 2 + bytes(sd.USER) + bytes(280)
    return sd.encode_form1(raw, user)


class CodecTests(unittest.TestCase):
    def test_edc_is_the_reflected_crc(self):
        data = os.urandom(300)
        self.assertEqual(sd.edc(data), bitwise_edc(data))

    def test_a_sector_carries_the_edc_of_subheader_and_data(self):
        user = os.urandom(sd.USER)
        raw = form1_sector(1234, user)
        self.assertEqual(raw[sd.USER_OFFSET:sd.USER_OFFSET + sd.USER], user)
        self.assertEqual(struct.unpack_from("<I", raw, 0x818)[0], bitwise_edc(raw[16:0x818]))
        self.assertTrue(sd.is_form1(raw))

    def test_re_encoding_is_stable_and_ignores_the_address_in_the_parity(self):
        user = os.urandom(sd.USER)
        raw = form1_sector(77, user)
        self.assertEqual(sd.encode_form1(raw, user), raw)
        other = form1_sector(78, user)
        self.assertEqual(raw[0x81C:], other[0x81C:])  # P and Q parity skip the header
        self.assertNotEqual(form1_sector(77, bytes(sd.USER))[0x81C:], raw[0x81C:])


class ExecutableTests(unittest.TestCase):
    def exe(self, size: int, tail: bytes = b"") -> bytes:
        header = bytearray(sd.HEADER_SIZE)
        struct.pack_into("<I", header, sd.TEXT_SIZE_OFFSET, size - sd.HEADER_SIZE)
        body = bytes([0x11]) * (size - sd.HEADER_SIZE - len(tail)) + tail
        return bytes(header) + body

    def test_a_zero_tail_is_cut_and_the_text_size_lowered(self):
        fitted, cut = sd.fit_executable(self.exe(0x1810, bytes(0x10)), 0x1800)
        self.assertEqual((len(fitted), cut), (0x1800, 0x10))
        self.assertEqual(struct.unpack_from("<I", fitted, sd.TEXT_SIZE_OFFSET)[0], 0x1000)

    def test_a_tail_that_is_not_zero_cannot_be_cut(self):
        with self.assertRaises(SystemExit):
            sd.fit_executable(self.exe(0x1810, bytes(0xF) + b"\x01"), 0x1800)

    def test_a_smaller_file_is_padded(self):
        fitted, cut = sd.fit_executable(self.exe(0x17F0), 0x1800)
        self.assertEqual((len(fitted), cut), (0x1800, 0))


class InsertPadTests(unittest.TestCase):
    def test_the_pad_goes_before_the_named_input(self):
        script = "    .main 0x80010000 :\n    {\n        a.o(.text*);\n        b.o(.text*);\n    }\n"
        out = sd.insert_pad(script, 0x100, "b.o(.text*)")
        self.assertIn("        a.o(.text*);\n        . += 0x100; /* shift_disc padding */\n        b.o(.text*);", out)
        with self.assertRaises(SystemExit):
            sd.insert_pad(script, 0x100, "c.o(.text*)")


if __name__ == "__main__":
    unittest.main()
