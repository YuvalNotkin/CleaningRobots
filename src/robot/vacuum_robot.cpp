#include "robot/vacuum_robot.hpp"

#include <iostream>

void VacuumRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE && state_ != RobotState::ARRIVED) {
        std::cout << "[Vacuum#" << name_ << "] busy, ignore move\n";
        return;
    }
    state_ = RobotState::MOVING;
    std::cout << "[Vacuum#" << name_ << "] start moving toward (" << dst.x << "," << dst.y << ")\n";
    pos_ = dst;
    state_ = RobotState::ARRIVED;
    publishStatus();
}

void VacuumRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::ARRIVED && state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    std::cout << "[Vacuum#" << name_ << "] start vacuuming\n";
    state_ = RobotState::IDLE;
    publishWorkCompleted(kind.empty() ? "VACUUM" : kind, true);
}

void VacuumRobot::stop() {
    state_ = RobotState::IDLE;
    publishStatus();
}
