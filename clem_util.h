static uint8_t g_hex_to_decimal[256] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x00-0x07 */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x08-0x0f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x10-0x17 */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x18-0x1f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x20-0x27 */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x28-0x2f */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,             /* 0x30-0x37 */
    0x08, 0x09, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x38-0x3f */
    0xff, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xff,             /* 0x40-0x4f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x48-0x4f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x50-0x5f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x58-0x5f */
    0xff, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0xff,             /* 0x60-0x6f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x68-0x6f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x70-0x7f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x78-0x7f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x80-0x8f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x88-0x8f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x90-0x9f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0x98-0x9f */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xa0-0xaf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xa8-0xaf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xb0-0xbf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xb8-0xbf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xc0-0xcf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xc8-0xcf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xd0-0xdf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xd8-0xdf */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xe0-0xef */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xe8-0xef */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,             /* 0xf0-0xff */
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff              /* 0xf8-0xff */
};

static char g_decimal_to_hex[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

/**
 * @brief Returns a pointer to the memory bank contents and if its a mega2 bank
 *
 * NOTE: the mega2 flag is only changed if the bank is a mega2 bank (and left
 * unchanged if not)
 *
 * @param clem
 * @param bank
 * @param mega2
 * @return uint8_t*
 */
static inline uint8_t* _clem_get_memory_bank(
    ClemensMachine* clem,
    uint8_t bank,
    bool* mega2
) {
    if (bank == 0xe0 || bank == 0xe1) {
        *mega2 = true;
        return clem->mega2_bank_map[bank & 0x1];
    }
    return clem->fpi_bank_map[bank];
}

static inline uint32_t _clem_calc_ns_step_from_clocks(
    clem_clocks_duration_t clocks_step,
    clem_clocks_duration_t clocks_step_reference
) {
    return  (uint32_t)(
        CLEM_MEGA2_CYCLE_NS * (uint64_t)clocks_step / clocks_step_reference);
}

static inline clem_clocks_duration_t _clem_calc_clocks_step_from_ns(
    unsigned ns,
    clem_clocks_duration_t clocks_step_reference
) {
    return  (clem_clocks_duration_t)(ns * clocks_step_reference) / (
        CLEM_MEGA2_CYCLE_NS);
}

static inline uint32_t _clem_calc_cycles_diff(
    uint32_t cycles_a,
    uint32_t cycles_b
) {
    uint32_t diff = cycles_b - cycles_a;
    if (cycles_b > cycles_a) {
        return cycles_b - cycles_a;
    } else {
        return cycles_b + (UINT32_MAX - cycles_a) + 1;
    }
}

static inline unsigned clem_util_timer_decrement(
    unsigned timer_ns,
    unsigned dt_ns
) {
    return (timer_ns - dt_ns < timer_ns) ? (timer_ns - dt_ns) : 0;
}

static inline unsigned clem_util_timer_increment(
    unsigned timer_ns,
    unsigned timer_max_ns,
    unsigned dt_ns
) {
    if (timer_ns + dt_ns > timer_ns) {
        timer_ns += dt_ns;
    } else {
        timer_ns = UINT32_MAX;
    }
    if (timer_ns > timer_max_ns) {
        timer_ns = timer_max_ns;
    }
    return timer_ns;
}

static bool _clem_util_hex_value(
    uint32_t* value,
    const char* start,
    const char* end
) {
    uint8_t hex;
    while (start != end) {
        *value <<= 4;
        hex = *start;
        if (hex == 0xff) return false;
        *value |= g_hex_to_decimal[hex];
        ++start;
    }
    return true;
}

