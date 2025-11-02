#include <memory>
#include <iostream>
#include "include/robot.hpp"
#include "include/robots_impl.hpp"  // your concrete DetectorRobot, VacuumRobot, WasherRobot
#include "include/registry.hpp"
#include "include/control_unit.hpp"
#include "include/bootstrap.hpp"

int main() {
    // 1) Build a registry and a small fleet
    RobotRegistry registry;
    auto d1 = std::make_shared<DetectorRobot>("d1", Position{  0,  0});
    auto d2 = std::make_shared<DetectorRobot>("d2", Position{100, 50});
    auto v1 = std::make_shared<VacuumRobot  >("v1", Position{ 20, 10});
    auto v2 = std::make_shared<VacuumRobot  >("v2", Position{300,300});
    auto w1 = std::make_shared<WasherRobot  >("w1", Position{ 15, -5});

    registry.add(d1); registry.add(d2);
    registry.add(v1); registry.add(v2);
    registry.add(w1);

    // 2) Prepare bootstrap feed entirely in main
    BootstrapFeed feed;
    feed.dirtSpots = {
        Position{12, 9},
        Position{30, 5},
        Position{250, 260}
    };

    // 3) Wire planner + control, seed, and run
    ControlUnit cu{registry};
    cu.seedFrom(feed);
    cu.runUntilIdle();

    std::cout << "Simulation completed.\n";
    return 0;
}
