#include "robot/detector_robot.hpp"

#include <iostream>

void DetectorRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE && state_ != RobotState::ARRIVED) {
        std::cout << "[Detector#" << name_ << "] busy, ignore move\n";
        return;
    }
    state_ = RobotState::MOVING;
    pos_ = dst;
    state_ = RobotState::ARRIVED;
    publishStatus();
}

void DetectorRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::ARRIVED && state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    std::cout << "[Detector#" << name_ << "] scanning for dirt\n";
    state_ = RobotState::IDLE;
    publishWorkCompleted(kind.empty() ? "DETECT" : kind, true);
}

void DetectorRobot::stop() {
    state_ = RobotState::IDLE;
    publishStatus();
}
