#include <iostream>
#include "include/control_unit.hpp"

// ---- simple helpers ----
void ControlUnit::moveRobot(RobotId id, Position dst) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->moveTo(dst);
}
void ControlUnit::startRobotWork(RobotId id, const std::string& kind) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->startWork(kind);
}
void ControlUnit::stopRobot(RobotId id) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->stop();
}
void ControlUnit::printRobots() const {
    auto printVec = [](const std::vector<std::shared_ptr<IRobot>>& vec) {
        for (const auto& r : vec) {
            std::cout << "[CU] Robot ID=" << r->id()
                      << " Type="  << static_cast<int>(r->type())
                      << " State=" << static_cast<int>(r->state()) << "\n";
        }
    };
    printVec(reg_.getByType(RobotType::DETECTOR));
    printVec(reg_.getByType(RobotType::VACUUM));
    printVec(reg_.getByType(RobotType::WASHER));
}

// ---- simulation flow ----
void ControlUnit::seedFrom(const BootstrapFeed& feed) {
    for (const auto& p : feed.dirtSpots) {
        planner_.enqueueDetect(p);
    }
}
void ControlUnit::runUntilIdle() {
    while (planner_.hasWork()) {
        (void)planner_.step(reg_);
    }
}
