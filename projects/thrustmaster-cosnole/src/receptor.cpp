#include <string.h>

#include "macros/unwrap.hpp"
#include "state-encoder.hpp"
#include "udp.hpp"

namespace {
auto send_command(const char sym, const ControlPacket::Wheel& wheel) -> void {
    switch(wheel.mode) {
    case ControlPacket::Wheel::Mode::Forward:
        print(sym, 'f');
        print(sym, 's', wheel.value);
        break;
    case ControlPacket::Wheel::Mode::Back:
        print(sym, 'r');
        print(sym, 's', wheel.value);
        break;
    case ControlPacket::Wheel::Mode::Brake:
        print(sym, 'b');
        break;
    case ControlPacket::Wheel::Mode::Coast:
        print(sym, 'c');
        break;
    }
}
} // namespace

auto main(const int argc, const char* const* argv) -> int {
    unwrap(sock, udp::create(0, 8080));
loop:
    auto packet = ControlPacket();
    if(read(sock.as_handle(), &packet, sizeof(packet)) != sizeof(packet)) {
        line_warn("failed to receive packet: ", strerror(errno));
    }
    send_command('l', packet.left);
    send_command('r', packet.right);
    goto loop;
}
