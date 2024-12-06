#pragma once
#include <cstdint>
#include <optional>
#include <vector>

struct Axis {
    int     num;
    int32_t min;
    int32_t max;
};

struct DeviceCaps {
    std::vector<Axis> axes;
    std::vector<int>  keys;

    static auto from_device(const int dev) -> std::optional<DeviceCaps>;

    auto find_axis(const int num) const -> const Axis*;
    auto debug_print() const -> void;
};

