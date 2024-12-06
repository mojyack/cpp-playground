#pragma once
#include "controller-state.hpp"

struct ControlPacket {
    struct Wheel {
        enum class Mode {
            Forward,
            Back,
            Brake,
            Coast,
        } mode;
        float value;
    };
    Wheel left;
    Wheel right;
};

auto encode_state(const ControllerState& state) -> ControlPacket;
