#include "include/robots_impl.hpp"
#include <iostream>

static const char* st(RobotState s){
    switch(s){
        case RobotState::IDLE: return "IDLE";
        case RobotState::MOVING: return "MOVING";
        case RobotState::WORKING: return "WORKING";
        case RobotState::ERROR: return "ERROR";
    }
    return "?";
}

// ----- Detector -----
void DetectorRobot::moveTo(Position) {}

void DetectorRobot::startWork(const std::string& kind) {
    std::cout << "[Detector#" << id_ << "] start " << kind << "\n";
    state_ = RobotState::WORKING;
    state_ = RobotState::IDLE;
    std::cout << "[Detector#" << id_ << "] done, state=" << st(state_) << "\n";
}

void DetectorRobot::stop() {
    state_ = RobotState::IDLE;
}

// ----- Vacuum -----
void VacuumRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) {
        std::cout << "[Vacuum#" << id_ << "] busy, ignore move\n";
        return;
    }
    state_ = RobotState::MOVING;
    pos_ = dst;
    std::cout << "[Vacuum#" << id_ << "] moved to (" << pos_.x << "," << pos_.y
              << "), state=" << st(state_) << "\n";
    state_ = RobotState::IDLE;
}


void VacuumRobot::startWork(const std::string& kind) {
    if (state_ == RobotState::IDLE) {
        state_ = RobotState::WORKING;
        std::cout << "[Vacuum#" << id_ << "] start " << kind << "\n";
        state_ = RobotState::IDLE;
        std::cout << "[Vacuum#" << id_ << "] done\n";
    }
}

void VacuumRobot::stop() { state_ = RobotState::IDLE; }

// ----- Washer -----
void WasherRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Washer#" << id_ << "] busy\n"; return; }
    state_ = RobotState::MOVING;
    pos_   = dst;
    std::cout << "[Washer#" << id_ << "] moved to (" << pos_.x << "," << pos_.y << ")\n";
    state_ = RobotState::IDLE;
}
void WasherRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) return;
    state_ = RobotState::WORKING;
    std::cout << "[Washer#" << id_ << "] start " << kind << "\n";
    state_ = RobotState::IDLE;
    std::cout << "[Washer#" << id_ << "] done\n";
}
void WasherRobot::stop() { state_ = RobotState::IDLE; }
