#include "pe1/aya.h"

/* Original stat queries from save/game_data.c, with native table pointers. */
int Aya_StatBinarySearch(int value, int *table) {
    int step;
    int index;
    int result;

    index = 0x40;
    step = 0x20;

    while (step != 0) {
        if (value >= table[index]) {
            index += step;
        } else if (value < table[index - 1]) {
            index -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    }

    index--;
    result = 0x62;
    if (index < 0x63) {
        result = index;
    }

    return result;
}

static inline int InterpolateSubLevel(int value, int *cursor) {
    int current_reg;
    int next_reg;
    int delta_reg;
    int numerator_reg;
    next_reg = cursor[1];
    current_reg = cursor[0];
    if (next_reg != current_reg) {
        delta_reg = (s32)((u32)value - (u32)current_reg);
        numerator_reg = (s32)((u32)delta_reg * 49u);
        return numerator_reg / (next_reg - current_reg);
    }
    return 0;
}

void Aya_StatQueryLevelAndSubLevel(int kind, int value, int *level_out, int *sublevel_out) {
    int saved_value;
    int *level_ptr;
    int *sublevel_ptr;
    int *table;
    int index;
    int step;
    int *loop_cursor;
    int *cursor;
    int sublevel;

    saved_value = value;
    level_ptr = level_out;
    sublevel_ptr = sublevel_out;
    table = Aya_GetStatGrowthTable(kind);

    index = 0x40;
    step = 0x20;
    do {
        loop_cursor = table + index;
        if (saved_value >= loop_cursor[0]) {
            index += step;
        } else if (saved_value < loop_cursor[-1]) {
            index -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    } while (step != 0);

    index--;
    step = 0x62;
    if (index < 0x63) {
        step = index;
    }
    index = step;

    if (index < 0x62) {
        cursor = table + index;
        sublevel = InterpolateSubLevel(saved_value, cursor);
    } else {
        sublevel = 0;
    }

    if (sublevel >= 0x31) {
        sublevel = 0x30;
    }

    if (level_ptr != 0) {
        *level_ptr = index;
    }
    if (sublevel_ptr != 0) {
        if (index < 0x62) {
            *sublevel_ptr = sublevel;
        } else {
            *sublevel_ptr = 0x30;
        }
    }
}

void Aya_StatQueryDistanceAndSubLevel(int kind, int value, int *distance_out, int *sublevel_out) {
    int saved_value;
    int *distance_ptr;
    int *sublevel_ptr;
    int *table;
    int index;
    int step;
    int *loop_cursor;
    int *cursor;
    int sublevel;
    int distance;

    saved_value = value;
    distance_ptr = distance_out;
    sublevel_ptr = sublevel_out;
    table = Aya_GetStatGrowthTable(kind);

    index = 0x40;
    step = 0x20;
    do {
        loop_cursor = table + index;
        if (saved_value >= loop_cursor[0]) {
            index += step;
        } else if (saved_value < loop_cursor[-1]) {
            index -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    } while (step != 0);

    index--;
    step = 0x62;
    if (index < 0x63) {
        step = index;
    }
    index = step;

    if (index < 0x62) {
        cursor = table + index;
        distance = cursor[1] - saved_value;
    } else {
        distance = 0;
    }

    if (index < 0x62) {
        cursor = table + index;
        sublevel = InterpolateSubLevel(saved_value, cursor);
    } else {
        sublevel = 0;
    }

    if (distance_ptr != 0) {
        *distance_ptr = distance;
    }
    if (sublevel_ptr != 0) {
        if (index < 0x62) {
            *sublevel_ptr = sublevel;
        } else {
            *sublevel_ptr = 0x30;
        }
    }
}

int Aya_StatOverflowQuery(int arg0, int value) {
    int *table;
    int step;
    int index;
    int result;

    table = Aya_GetStatGrowthTable(arg0);
    index = 0x40;
    step = 0x20;

    while (step != 0) {
        if (value >= table[index]) {
            index += step;
        } else if (value < table[index - 1]) {
            index -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    }

    index--;
    result = 0x62;
    if (index < 0x63) {
        result = index;
    }

    if (result <= 0) {
        return 0;
    }
    return value - table[result - 1];
}

