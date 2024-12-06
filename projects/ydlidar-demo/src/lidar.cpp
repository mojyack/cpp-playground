#include <cstring>

#include <CYdLidar.h>

#include "macros/assert.hpp"
#include "util/pair-table.hpp"

auto init_lidar(CYdLidar& laser) -> bool {
    ydlidar::os_init();
    ensure(ydlidar::os_isOk());

    auto port = "/dev/ttyUSB0";
    ensure(laser.setlidaropt(LidarPropSerialPort, port, std::strlen(port)));

    auto opts_int = make_pair_table<uint32_t, int>({
        {LidarPropSerialBaudrate, 128000},
        {LidarPropLidarType, TYPE_TRIANGLE},
        {LidarPropDeviceType, YDLIDAR_TYPE_SERIAL},
        {LidarPropSampleRate, 5},
        {LidarPropAbnormalCheckCount, 4},
        {LidarPropIntenstiyBit, 10},
    });
    for(auto& opt : opts_int.array) {
        ensure(laser.setlidaropt(opt.first, &opt.second, sizeof(int)));
    }

    auto opts_bool = make_pair_table<uint32_t, bool>({
        {LidarPropFixedResolution, false},
        {LidarPropReversion, false},
        {LidarPropInverted, false},
        {LidarPropAutoReconnect, true},
        {LidarPropSingleChannel, true},
        {LidarPropIntenstiy, false},
        {LidarPropSupportMotorDtrCtrl, true},
        {LidarPropSupportHeartBeat, false},
    });
    for(auto& opt : opts_bool.array) {
        ensure(laser.setlidaropt(opt.first, &opt.second, sizeof(bool)));
    }

    auto opts_float = make_pair_table<uint32_t, float>({
        {LidarPropMaxAngle, 180.0}, // degree
        {LidarPropMinAngle, -180.0},
        {LidarPropMaxRange, 64.0}, // metre
        {LidarPropMinRange, 0.05},
        {LidarPropScanFrequency, 16.0}, // Hz
    });
    for(auto& opt : opts_float.array) {
        ensure(laser.setlidaropt(opt.first, &opt.second, sizeof(float)));
    }

    // disable glass noise filtering
    laser.enableGlassNoise(false);
    // disable sunlight noise filtering
    laser.enableSunNoise(false);
    // get baseboard device information first
    laser.setBottomPriority(true);
    // enable debugging
    laser.setEnableDebug(false);

    ensure(laser.initialize(), laser.DescribeError());
    ensure(laser.turnOn(), laser.DescribeError());
    if(auto str = std::string(); laser.getUserVersion(str)) {
        print("user version: ", str);
    }
    if(auto info = device_info(); laser.getDeviceInfo(info, EPT_Module)) {
        print("module info");
        print("  model: ", int(info.model));
        print("  firmware version: ", int(info.firmware_version));
        print("  hardware version: ", int(info.hardware_version));
    }
    if(auto info = device_info(); laser.getDeviceInfo(info, EPT_Base)) {
        print("base info");
        print("model: ", int(info.model));
        print("firmware version: ", int(info.firmware_version));
        print("hardware version: ", int(info.hardware_version));
    }

    return true;
}
