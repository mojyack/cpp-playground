#include <cmath>
#include <cstring>
#include <numeric>

#include <CYdLidar.h>
#include <coop/thread.hpp>

#include "gawl/misc.hpp"
#include "gawl/wayland/application.hpp"
#include "lidar.hpp"
#include "macros/unwrap.hpp"
#include "payloader.hpp"
#include "udp.hpp"

class Callbacks : public gawl::WindowCallbacks {
  private:
    int count = 0;

  public:
    auto refresh() -> void override;

    auto close() -> void override {
        application->quit();
    }

    auto on_created(gawl::Window* window) -> coop::Async<bool> override;
};

auto points = std::vector<Point>();
auto window = (gawl::Window*)(nullptr);

auto Callbacks::refresh() -> void {
    // gawl::clear_screen({0, 0, 0, 1});
    gawl::mask_alpha();

    const auto [width, height] = window->get_window_size();
    const auto center_x        = width / 2.0;
    const auto center_y        = height / 2.0;
    const auto max_radius      = std::min(center_x, center_y) * 0.8;
    const auto angle_offset    = 170 / 360.0 * (2 * std::numbers::pi);
    gawl::draw_rect(*window, {{0, 0}, {1. * width, 1. * height}}, {0, 0, 0, 0.5});

    // for(const auto& point : points) {
    //     const auto angle = point.angle + std::numbers::pi;
    //     const auto x     = angle / (2 * std::numbers::pi) * width;
    //     const auto y     = height * (1 - point.distance / 10.0);
    //     gawl::draw_rect(*window, {{x - 2, y - 2}, {x + 2, y + 2}}, {0, 1, 0, 0.5});
    // }

    for(const auto& point : points) {
        // print("refresh ", point.angle, " ", point.range);
        const auto angle = point.angle + angle_offset;
        // const auto radius = point.angle / scan.config.max_range * max_radius;
        const auto radius = point.distance / 10 * max_radius;
        const auto x      = center_x + std::cos(angle) * radius;
        const auto y      = center_y + std::sin(angle) * radius;
        gawl::draw_rect(*window, {{x - 2, y - 2}, {x + 2, y + 2}}, {1, 1, 1, 1});
    }

    gawl::draw_rect(*window, {{center_x - 2, center_y - 2}, {center_x + 2, center_y + 2}}, {1, 0, 0, 1});

    count += 1;
}

auto Callbacks::on_created(gawl::Window* /*window*/) -> coop::Async<bool> {
    ::window = window;
    co_return true;
}

auto laser = CYdLidar();

auto sensor_task() -> coop::Async<void> {
    auto scan = LaserScan();
loop:
    co_ensure_v(ydlidar::os_isOk());
    co_ensure_v(co_await coop::run_blocking([&]() { return laser.doProcessSimple(scan); }));
    print("received ", scan.points.size(), " points, ", scan.scanFreq, " Hz");
    points.resize(scan.points.size());
    for(auto i = size_t(0); i < scan.points.size(); i += 1) {
        points[i].angle    = scan.points[i].angle;
        points[i].distance = scan.points[i].range;
    }
    if(window != nullptr) {
        window->refresh();
    }
    goto loop;
}

auto receive_task() -> coop::Async<void> {
    auto depay  = Depayloader();
    auto buffer = std::vector<std::byte>(1500);

    co_unwrap_v(sock, udp::create(0, 8081));
loop:
    const auto result = co_await coop::wait_for_file(sock.as_handle(), true, false);
    co_ensure_v(result.read && !result.error);
    const auto size = read(sock.as_handle(), buffer.data(), buffer.size());
    co_ensure_v(size > 0);
    if(auto ret = depay.payload_to_points({buffer.data(), size_t(size)})) {
        points = std::move(*ret);
    }
    if(window != nullptr) {
        window->refresh();
    }
    goto loop;
}

auto main() -> int {
    // ensure(init_lidar(laser));

    auto runner = coop::Runner();
    auto app    = gawl::WaylandApplication();
    auto cbs    = std::shared_ptr<Callbacks>(new Callbacks());
    runner.push_task(app.run());
    runner.push_task(app.open_window({.manual_refresh = true}, std::move(cbs)));
    // runner.push_task(sensor_task());
    runner.push_task(receive_task());
    runner.run();
}
