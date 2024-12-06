#include "state-encoder.hpp"

auto encode_state(const ControllerState& state) -> ControlPacket {
    // 1. if brake pressed
    //   1-1. if accel, back
    //   1-2. if not, brake
    // 2. if accel pressed, forward
    // 3. coast

    using Mode = ControlPacket::Wheel::Mode;

    constexpr auto brake_threshold = 0.2;

    auto mode = Mode();
    if(state.brake >= brake_threshold) {
        mode = state.accel > 0 ? Mode::Back : Mode::Brake;
    } else if(state.accel > 0) {
        mode = Mode::Forward;
    } else {
        mode = Mode::Coast;
    }

    auto packet       = ControlPacket();
    packet.left.mode  = mode;
    packet.right.mode = mode;

    if(mode == Mode::Forward || mode == Mode::Back) {
        auto lvalue = state.accel;
        auto rvalue = state.accel;
        if(state.steering > 0) {
            rvalue *= 1.0 - state.steering;
        } else {
            lvalue *= 1.0 + state.steering;
        }
        packet.right.value = rvalue;
        packet.left.value  = lvalue;
    }

    return packet;
};
