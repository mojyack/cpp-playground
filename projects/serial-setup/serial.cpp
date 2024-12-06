#include <array>
#include <cstring>

#include <fcntl.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define termios asmtermios // hack to avoid confliction of struct termios
#include <asm/termbits.h>
#undef termios

#include "macros/assert.hpp"
#include "serial.hpp"

auto configure_serial_port(const int fd, const int baudrate) -> bool {
    auto tio = termios{};
    memset(&tio, 0, sizeof(tio));
    tio.c_cflag = CREAD | CLOCAL | CS8 | BOTHER;
    // ensure(cfsetispeed(&tio, baudrate) == 0, strerror(errno));
    // ensure(cfsetospeed(&tio, baudrate) == 0);
    cfmakeraw(&tio);
    tio.c_ispeed = baudrate;
    tio.c_ospeed = baudrate;
    ensure(tcsetattr(fd, TCSANOW, &tio) == 0);
    ensure(ioctl(fd, TCSETS, &tio) == 0, "setup tty failed: ", strerror(errno));
    return true;
}

auto configure_serial_port_non_standard(const int fd, const int baudrate) -> bool {
    {
        auto tio = termios{};
        ensure(tcgetattr(fd, &tio) == 0);
        cfmakeraw(&tio);
        tio.c_cflag |= CLOCAL | CREAD;
        tio.c_cc[VTIME] = 0;
        tio.c_cc[VMIN]  = 0;

        tio.c_cflag &= ~CSIZE;
        tio.c_cflag |= CS8;

        tio.c_iflag &= ~(PARMRK | INPCK);
        tio.c_iflag |= IGNPAR;
        tio.c_cflag &= ~PARENB;

        tio.c_cflag &= ~CSTOPB;

        tio.c_cflag &= ~CRTSCTS;
        tio.c_iflag &= ~(IXON | IXOFF | IXANY);

        ensure(tcsetattr(fd, TCSANOW, &tio) == 0);
    }

    auto tio = termios2{};
    ensure(ioctl(fd, TCGETS2, &tio) == 0, int(errno));
    // tio.c_cflag = CREAD | CLOCAL | CS8 | BOTHER;
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = baudrate;
    tio.c_ospeed = baudrate;
    ensure(ioctl(fd, TCSETS2, &tio) == 0);

    ensure(ioctl(fd, TCGETS2, &tio) == 0, int(errno));
    print("current baudrate ", tio.c_ispeed, " ", tio.c_ospeed);
    return true;
}

auto configure_serial_port_legacy(int fd, int baudrate) -> bool {
    {
        struct termios term;
        ensure(tcgetattr(fd, &term) == 0);
        term.c_cflag &= ~(CBAUD | CBAUDEX);
        term.c_cflag |= B38400;
        ensure(tcsetattr(fd, TCSANOW, &term) == 0);
    }

    auto serial = serial_struct();
    ensure(ioctl(fd, TIOCGSERIAL, &serial) == 0);
    serial.flags &= ~ASYNC_SPD_MASK;
    serial.flags |= (ASYNC_SPD_CUST /* | ASYNC_LOW_LATENCY*/);
    serial.custom_divisor = serial.baud_base / baudrate;
    print(serial.baud_base, " ", baudrate);
    ensure(serial.custom_divisor != 0);
    if(serial.custom_divisor * baudrate != serial.baud_base) {
    }
    ensure(ioctl(fd, TIOCSSERIAL, &serial) == 0);

    // auto info = serial_struct();
    // ensure(ioctl(fd, TIOCGSERIAL, &info) == 0);
    // print("current ", info.baud_base, " reminder ", info.baud_base % baudrate);
    // info.flags = ASYNC_SPD_CUST | ASYNC_LOW_LATENCY;
    // ser_info.custom_divisor = ser_info.baud_base / CUST_BAUD_RATE;
    // if(ioctl(ser_dev, TIOCSSERIAL, &ser_info) < 0)
    return true;
}

auto flush_input_buffer(const int fd) -> bool {
    ensure(tcflush(fd, TCIFLUSH) == 0);
    return true;
    //    const auto flag = fcntl(fd, F_GETFL);
    //    ensure(fcntl(fd, F_SETFL, flag | O_NONBLOCK) == 0);
    //    auto buf = std::array<std::byte, 4096>();
    // loop:
    //    if(read(fd, buf.data(), buf.size()) > 0) {
    //        goto loop;
    //    }
    //    ensure(errno == EAGAIN, "unexpected error: ", strerror(errno));
    //    ensure(fcntl(fd, F_SETFL, flag) == 0);
    //    return true;
}

auto set_dtr(const int fd, const bool dtr) -> bool {
    auto flags = 0;
    ensure(ioctl(fd, TIOCMGET, &flags) == 0);
    if(dtr) {
        flags |= TIOCMGET;
    } else {
        flags &= ~TIOCMGET;
    }
    ensure(ioctl(fd, TIOCMSET, &flags) == 0);
    return true;
}
