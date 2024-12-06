#include <array>

#include <errno.h>
#include <linux/uinput.h>
#include <string.h>

#include "caps.hpp"
#include "macros/unwrap.hpp"

namespace {
template <int event>
struct EventBits {
    template <size_t bits>
    constexpr static auto bits_to_bytes = (bits - 1) / 8 + 1;

    constexpr static auto code_max_table = std::array{
        SYN_MAX, // 0x00
        KEY_MAX, // 0x01
        REL_MAX, // 0x02
        ABS_MAX, // 0x03
        MSC_MAX, // 0x04
        SW_MAX,  // 0x05,
    };

    std::array<uint8_t, bits_to_bytes<code_max_table[event]>> data;

    static auto from_device(const int fd) -> std::optional<EventBits<event>> {
        auto  bits = EventBits<event>();
        auto& raw  = bits.data;
        ensure(ioctl(fd, EVIOCGBIT(event, raw.size()), raw.data()) == int(raw.size()), strerror(errno));
        return bits;
    }

    auto size() const -> size_t {
        return code_max_table[event];
    }

    auto operator[](const size_t index) const -> bool {
        const auto byte = data[index / 8];
        return ((byte >> (index % 8)) & 1) != 0;
    }
};
} // namespace

auto DeviceCaps::from_device(const int dev) -> std::optional<DeviceCaps> {
    auto ret = DeviceCaps();
    {
        unwrap(bits, EventBits<EV_ABS>::from_device(dev));
        for(auto i = 0u; i < bits.size(); i += 1) {
            if(!bits[i]) {
                continue;
            }
            auto info = input_absinfo();
            ioctl(dev, EVIOCGABS(i), &info);
            ret.axes.emplace_back(Axis{
                .num = int(i),
                .min = info.minimum,
                .max = info.maximum,
            });
        }
    }
    {
        unwrap(bits, EventBits<EV_KEY>::from_device(dev));
        for(auto i = 0u; i < bits.size(); i += 1) {
            if(!bits[i]) {
                continue;
            }
            ret.keys.push_back(i);
        }
    }
    return ret;
}

auto DeviceCaps::find_axis(const int num) const -> const Axis* {
    const auto i = std::ranges::find_if(axes, [num](const auto& axes) { return axes.num == num; });
    return i == axes.end() ? nullptr : &*i;
}

auto DeviceCaps::debug_print() const -> void {
    for(const auto& axis : axes) {
        printf("axis %d: min=%d max=%d\n", axis.num, axis.min, axis.max);
    }
    printf("keys: ");
    for(const auto key : keys) {
        printf("%d ", key);
    }
    printf("\n");
}
