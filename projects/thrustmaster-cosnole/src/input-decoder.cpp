#include "input-decoder.hpp"
#include "macros/unwrap.hpp"

auto InputDecoder::create(const DeviceCaps& caps, InputProfile profile) -> std::optional<InputDecoder> {
    unwrap(steering_caps, caps.find_axis(profile.steering_axis));
    unwrap(accel_caps, caps.find_axis(profile.accel_axis));
    unwrap(brake_caps, caps.find_axis(profile.brake_axis));
    return InputDecoder{
        .profile       = profile,
        .steering_caps = steering_caps,
        .accel_caps    = accel_caps,
        .brake_caps    = brake_caps,
    };
}

auto InputDecoder::decode_input(const input_event& event, ControllerState& state) -> bool {
    auto calc_value = [](const int32_t raw_value, const Axis& caps, const double threshold, const bool invert) -> double {
        const auto value         = (invert ? caps.max - raw_value : raw_value) - caps.min; // to 0 base
        const auto range         = caps.max - caps.min;
        const auto raw_threshold = range * threshold;
        if(value < raw_threshold) {
            return 0;
        } else {
            return (value - raw_threshold) / (range - raw_threshold);
        }
    };

    if(event.type != EV_ABS) {
        return false;
    }

    if(event.code == profile.steering_axis) {
        const auto value = event.value - steering_caps.min; // to 0 base
        const auto range = steering_caps.max - steering_caps.min;
        state.steering   = (value - range / 2.0) / range * 2;
    } else if(event.code == profile.accel_axis) {
        state.accel = calc_value(event.value, accel_caps, profile.accel_threshold, profile.invert_accel);
    } else if(event.code == profile.brake_axis) {
        state.brake = calc_value(event.value, brake_caps, profile.brake_threshold, profile.invert_brake);
    } else {
        return false;
    }
    return true;
}
