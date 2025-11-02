#include "include/robots_impl.hpp"
#include <iostream>

static const char* st(RobotState s){
    switch(s){
        case RobotState::IDLE:    return "IDLE";
        case RobotState::MOVING:  return "MOVING";
        case RobotState::WORKING: return "WORKING";
        case RobotState::ERROR:   return "ERROR";
    }
    return "?";
}

// ----- Detector -----
void DetectorRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Detector#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    pos_   = dst;
    std::cout << "[Detector#" << name_ << "] moved to (" << pos_.x << "," << pos_.y
              << "), state=" << st(state_) << "\n";
    state_ = RobotState::IDLE;
}

void DetectorRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { std::cout << "[Detector#" << name_ << "] busy, ignore work\n"; return; }
    state_ = RobotState::WORKING;
    std::cout << "[Detector#" << name_ << "] start " << kind << "\n";
    state_ = RobotState::IDLE;
    std::cout << "[Detector#" << name_ << "] done, state=" << st(state_) << "\n";
}

void DetectorRobot::stop() { state_ = RobotState::IDLE; }

// ----- Vacuum -----
void VacuumRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Vacuum#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    pos_   = dst;
    std::cout << "[Vacuum#" << name_ << "] moved to (" << pos_.x << "," << pos_.y
              << "), state=" << st(state_) << "\n";
    state_ = RobotState::IDLE;
}

void VacuumRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { std::cout << "[Vacuum#" << name_ << "] busy, ignore work\n"; return; }
    state_ = RobotState::WORKING;
    std::cout << "[Vacuum#" << name_ << "] start " << kind << "\n";
    state_ = RobotState::IDLE;
    std::cout << "[Vacuum#" << name_ << "] done\n";
}

void VacuumRobot::stop() { state_ = RobotState::IDLE; }

// ----- Washer -----
void WasherRobot::moveTo(Position dst) {
    if (state_ != RobotState::IDLE) { std::cout << "[Washer#" << name_ << "] busy, ignore move\n"; return; }
    state_ = RobotState::MOVING;
    pos_   = dst;
    std::cout << "[Washer#" << name_ << "] moved to (" << pos_.x << "," << pos_.y << ")\n";
    state_ = RobotState::IDLE;
}

void WasherRobot::startWork(const std::string& kind) {
    if (state_ != RobotState::IDLE) { std::cout << "[Washer#" << name_ << "] busy, ignore work\n"; return; }
    state_ = RobotState::WORKING;
    std::cout << "[Washer#" << name_ << "] start " << kind << "\n";
    state_ = RobotState::IDLE;
    std::cout << "[Washer#" << name_ << "] done\n";
}

void WasherRobot::stop() { state_ = RobotState::IDLE; }
