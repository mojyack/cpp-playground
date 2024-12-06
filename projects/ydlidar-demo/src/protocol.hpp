#pragma once
#include <cstdint>

static_assert(sizeof(float) == 4);

struct Point {
    float angle;
    float distance;
};

struct Header {
    uint8_t frame;
    uint8_t serial_num;
    uint8_t serial_end;
    uint8_t points_len;
    // Point points[];
};
