#pragma once
#include <linux/input.h>

#include "caps.hpp"
#include "controller-state.hpp"

struct InputProfile {
    uint16_t steering_axis;
    uint16_t accel_axis;
    uint16_t brake_axis;
    double   accel_threshold;
    double   brake_threshold;
    bool     invert_accel;
    bool     invert_brake;
};
struct InputDecoder {
    InputProfile profile;
    Axis         steering_caps;
    Axis         accel_caps;
    Axis         brake_caps;

    static auto create(const DeviceCaps& caps, InputProfile profile) -> std::optional<InputDecoder>;

    auto decode_input(const input_event& event, ControllerState& state) -> bool;
};

