#pragma once

struct ControllerState {
    double steering = 0.0; // -1.0 ~ 1.0, cw
    double accel    = 0.0; // 0.0 ~ 1.0
    double brake    = 0.0; // 0.0 ~ 1.0
};

