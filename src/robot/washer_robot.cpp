#include "robot/washer_robot.hpp"

#include <iostream>

void WasherRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE && state_ != RobotState::ARRIVED) {
        std::cout << "[Washer#" << name_ << "] busy, ignore move\n";
        return;
    }
    state_ = RobotState::MOVING;
    std::cout << "[Washer#" << name_ << "] start moving toward (" << dst.x << "," << dst.y << ")\n";
    pos_ = dst;
    state_ = RobotState::ARRIVED;
    publishStatus();
}

void WasherRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::ARRIVED && state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    std::cout << "[Washer#" << name_ << "] start washing\n";
    state_ = RobotState::IDLE;
    publishWorkCompleted(kind.empty() ? "WASH" : kind, true);
}

void WasherRobot::stop() {
    state_ = RobotState::IDLE;
    publishStatus();
}
