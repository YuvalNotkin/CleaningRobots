#include "include/control_unit.hpp"
#include <iostream>
#include "control_unit.hpp"

void ControlUnit::moveRobot(RobotId id, Position dst) {
    auto r = reg_->getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->moveTo(dst);
}

void ControlUnit::startRobotWork(RobotId id, const std::string& kind) {
    auto r = reg_->getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->startWork(kind);
}

void ControlUnit::stopRobot(RobotId id) {
    auto r = reg_->getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->stop();
}

void ControlUnit::printRobots() const {
    for (auto& r : reg_->all()) {
        std::cout << "[CU] Robot ID=" << r->id()
                  << " Type=" << static_cast<int>(r->type())
                  << " State=" << static_cast<int>(r->state()) << "\n";
    }
}

