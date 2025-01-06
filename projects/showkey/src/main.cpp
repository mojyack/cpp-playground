#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

#include "macros/assert.hpp"
#include "util/argument-parser.hpp"

namespace {
auto keycode_to_symbol(const uint16_t code) -> const char* {
#define SYM(num, str) \
    case KEY_##num:   \
        return str

    switch(code) {
        SYM(1, "1");
        SYM(2, "2");
        SYM(3, "3");
        SYM(4, "4");
        SYM(5, "5");
        SYM(6, "6");
        SYM(7, "7");
        SYM(8, "8");
        SYM(9, "9");
        SYM(0, "0");
        SYM(A, "a");
        SYM(B, "b");
        SYM(C, "c");
        SYM(D, "d");
        SYM(E, "e");
        SYM(F, "f");
        SYM(G, "g");
        SYM(H, "h");
        SYM(I, "i");
        SYM(J, "j");
        SYM(K, "k");
        SYM(L, "l");
        SYM(M, "m");
        SYM(N, "n");
        SYM(O, "o");
        SYM(P, "p");
        SYM(Q, "q");
        SYM(R, "r");
        SYM(S, "s");
        SYM(T, "t");
        SYM(U, "u");
        SYM(V, "v");
        SYM(W, "w");
        SYM(X, "x");
        SYM(Y, "y");
        SYM(Z, "z");
        SYM(MINUS, "-");
        SYM(EQUAL, "=");
        SYM(LEFTBRACE, "{");
        SYM(RIGHTBRACE, "}");
        SYM(SEMICOLON, ";");
        SYM(APOSTROPHE, "'");
        SYM(GRAVE, "`");
        SYM(BACKSLASH, "\\");
        SYM(COMMA, ",");
        SYM(DOT, ".");
        SYM(SLASH, "/");
    default:
        return nullptr;
    }
}

auto keycode_to_str(const uint16_t code) -> const char* {
    switch(code) {
#include "code2str.txt"
    default:
        return "unknown";
    }
}

auto capture_events(const int fd) -> void {
    auto shift = false;
    auto alt   = false;
    auto ctrl  = false;
    auto event = input_event();
loop:
    ensure(read(fd, &event, sizeof(event)) == sizeof(event));
    // printf("[%ld.%ld] %d %d %d\n", event.time.tv_sec, event.time.tv_usec, event.type, event.code, event.value);

    if(event.type != EV_KEY) {
        goto loop;
    }
    if(event.value == 2) { // repeat
        goto loop;
    }

    switch(event.code) {
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
        shift = event.value == 1;
        goto loop;
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
        alt = event.value == 1;
        goto loop;
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
        ctrl = event.value == 1;
        goto loop;
    }

    if(event.value != 1) { // not press
        goto loop;
    }

    std::cout << '[';
    std::cout << (shift ? 'S' : ' ');
    std::cout << (alt ? 'A' : ' ');
    std::cout << (ctrl ? 'C' : ' ');
    std::cout << ']' << ' ';

    const auto sym = keycode_to_symbol(event.code);
    if(sym != nullptr) {
        std::cout << sym;
    } else {
        const auto str = keycode_to_str(event.code);
        std::cout << '(' << str + 4 << ')'; // +4 to skip "key_"
    }
    std::cout << std::endl;

    goto loop;
}
} // namespace

auto device_file = "-";

auto main(const int argc, const char* const* argv) -> int {
    {
        auto help   = false;
        auto parser = args::Parser<>();
        parser.kwarg(&device_file, {"-d", "--device"}, "FILE", "path to device file or '-' for stdin", {.state = args::State::DefaultValue});
        parser.kwflag(&help, {"-h", "--help"}, "print this help message", {.no_error_check = true});
        if(!parser.parse(argc, argv) || help) {
            print("usage: evvis ", parser.get_help());
            return help;
        }
    }

    auto fd = int();
    if(device_file == std::string_view("-")) {
        fd = fileno(stdin);
    } else {
        fd = open(device_file, O_RDONLY);
        ensure(fd >= 0);
    }
    capture_events(fd);
    return 0;
}
