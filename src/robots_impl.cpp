#include "include/robots_impl.hpp"
#include <iostream>

// ----- Detector -----
void DetectorRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Detector#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    pos_ = dst;
    state_ = RobotState::IDLE;
}

void DetectorRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    state_ = RobotState::IDLE;
}

void DetectorRobot::stop() { state_ = RobotState::IDLE; }

// ----- Vacuum -----
void VacuumRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Vacuum#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    std::cout << "[Vacuum#" << name_ << "] start moving toward (" << dst.x << "," << dst.y << ")\n";
    pos_ = dst;
    state_ = RobotState::IDLE;
}

void VacuumRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    state_ = RobotState::IDLE;
}

void VacuumRobot::stop() { state_ = RobotState::IDLE; }

// ----- Washer -----
void WasherRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Washer#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    std::cout << "[Washer#" << name_ << "] start moving toward (" << dst.x << "," << dst.y << ")\n";
    pos_   = dst;
    state_ = RobotState::IDLE;
}

void WasherRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { return; }
    state_ = RobotState::WORKING;
    state_ = RobotState::IDLE;
}

void WasherRobot::stop() { state_ = RobotState::IDLE; }
