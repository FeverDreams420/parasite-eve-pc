#include "common.h"
#include "pe1/psyq_press.h"

/* Original LIBPRESS VLC_C decoder. Pointers in its suspended state are native;
 * the bitstream and lookup tables keep their little-endian PS1 format. */
static u32 g_MdecVlc2SizeLimit = 0x00ffffff;
static struct {
    const u8 *input;
    u8 *output, *end;
    u32 bits, used, quant, block, predictor[3];
    int suspended;
} g_MdecVlc2SavedState;

static u32 MdecVlc_Read16(const u8 *p) {
    return p[0] | (u32)p[1] << 8;
}

static u32 MdecVlc_Read32(const u8 *p) {
    return MdecVlc_Read16(p) | MdecVlc_Read16(p + 2) << 16;
}

static void MdecVlc_Write16(u8 *p, u32 value) {
    p[0] = (u8)value;
    p[1] = (u8)(value >> 8);
}

int DecDCTvlcSize2(int size) {
    u32 previous = g_MdecVlc2SizeLimit;
    g_MdecVlc2SizeLimit = size > 1 ? (u32)size * 2 : 0x00ffffff;
    return (int)previous;
}

int DecDCTvlc2(void *input, void *decoded, void *vlc) {
    const u8 *table = vlc;
    const u8 *source;
    u8 *output, *chunk_start;
    u32 bits, used, quant, block, predictor[3];
    u32 code, packed, extra, count, dc, length;
    if (!table) return -1;
    if (input) {
        const u8 *frame = input;
        if (!decoded) return -1;
        source = frame + 12;
        output = decoded;
        chunk_start = output;
        u32 header = MdecVlc_Read32(frame);
        /* The advertised MDEC payload counts 32-bit words. */
        g_MdecVlc2SavedState.end = output + 4 + (header & 0xffff) * 4;
        MdecVlc_Write16(output, header);
        MdecVlc_Write16(output + 2, header >> 16);
        output += 2;
        bits = MdecVlc_Read16(frame + 8) << 16 | MdecVlc_Read16(frame + 10);
        used = 0;
        quant = MdecVlc_Read16(frame + 4) << 10;
        block = MdecVlc_Read16(frame + 6) >= 3 ? 1 : 0;
        predictor[0] = predictor[1] = predictor[2] = 0;
        g_MdecVlc2SavedState.suspended = 0;
        goto dc_coefficient;
    }
    if (!g_MdecVlc2SavedState.suspended) return -1;
    source = g_MdecVlc2SavedState.input;
    output = g_MdecVlc2SavedState.output;
    chunk_start = output;
    bits = g_MdecVlc2SavedState.bits;
    used = g_MdecVlc2SavedState.used;
    quant = g_MdecVlc2SavedState.quant;
    block = g_MdecVlc2SavedState.block;
    for (unsigned i = 0; i < 3; ++i) predictor[i] = g_MdecVlc2SavedState.predictor[i];
    goto ac_coefficient;

dc_coefficient:
    dc = bits >> 22;
    output += 2;
    if (dc == (block ? 0x3ffu : 0x1ffu)) goto frame_complete;
    if (output + 2 > g_MdecVlc2SavedState.end) goto invalid;
    if (block) {
        const u8 *entry = table + (block < 3 ? 0x400 : 0) + (bits >> 24) * 4;
        length = MdecVlc_Read16(entry);
        count = MdecVlc_Read16(entry + 2);
        bits <<= length & 31;
        dc = 0;
        if (count) {
            dc = bits >> ((32 - count) & 31);
            if (!(bits & 0x80000000u)) dc -= 0xffffffffu >> ((32 - count) & 31);
            bits <<= count & 31;
            used += count;
        }
        used += length;
        unsigned component = block < 3 ? block - 1 : 2;
        predictor[component] += dc;
        code = quant | ((predictor[component] << 2) & 0x3ff);
        if (++block == 7) block = 1;
    } else {
        code = quant | dc;
        bits <<= 10;
        used += 10;
    }
    if (used & 16) {
        used &= 15;
        bits |= MdecVlc_Read16(source) << used;
        source += 2;
    } else used &= 15;
    MdecVlc_Write16(output, code);
    /* LIBPRESS checks its work limit at DC boundaries, after writing DC. */
    if ((size_t)(output - chunk_start) >= (size_t)g_MdecVlc2SizeLimit * 2) {
        output += 2;
        g_MdecVlc2SavedState.input = source;
        g_MdecVlc2SavedState.output = output;
        g_MdecVlc2SavedState.bits = bits;
        g_MdecVlc2SavedState.used = used;
        g_MdecVlc2SavedState.quant = quant;
        g_MdecVlc2SavedState.block = block;
        for (unsigned i = 0; i < 3; ++i) g_MdecVlc2SavedState.predictor[i] = predictor[i];
        g_MdecVlc2SavedState.suspended = 1;
        return 1;
    }
    output += 2;

ac_coefficient:
    {
        const u8 *entry = table + 0x800 + (bits >> 19) * 8;
        packed = MdecVlc_Read32(entry);
        extra = 0;
        if (packed) extra = MdecVlc_Read32(entry + 4);
        else {
            bits <<= 8;
            used += 8;
            if (used & 16) {
                used &= 15;
                bits |= MdecVlc_Read16(source) << used;
                source += 2;
            } else used &= 15;
            packed = MdecVlc_Read32(table + 0x10800 + (bits >> 23) * 4);
        }
    }
    count = packed & 255;
    bits <<= count & 31;
    used += count;
    if (used & 16) {
        used &= 15;
        bits |= MdecVlc_Read16(source) << used;
        source += 2;
    } else used &= 15;
    code = packed >> 16;
    for (unsigned i = 0; ; ++i) {
        if (code == 0x7c1f) {
            if (output + 2 > g_MdecVlc2SavedState.end) goto invalid;
            MdecVlc_Write16(output, bits >> 16);
            output += 2;
            bits = bits << 16 | MdecVlc_Read16(source) << used;
            source += 2;
            goto ac_coefficient;
        }
        if (output + 2 > g_MdecVlc2SavedState.end) goto invalid;
        MdecVlc_Write16(output, code);
        if (code == 0xfe00) goto dc_coefficient;
        output += 2;
        if (i == 2 || !extra) goto ac_coefficient;
        code = i == 0 ? extra & 0xffff : extra >> 16;
        if (i == 1 && !code) goto ac_coefficient;
    }

frame_complete:
    while (output < g_MdecVlc2SavedState.end) {
        MdecVlc_Write16(output, 0xfe00);
        output += 2;
    }
    g_MdecVlc2SavedState.suspended = 0;
    return 0;
invalid:
    g_MdecVlc2SavedState.suspended = 0;
    return -1;
}
