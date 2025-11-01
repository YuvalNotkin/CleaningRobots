#pragma once
#include "registry.hpp"

class ControlUnit {
public:
    explicit ControlUnit(RobotRegistry* reg) : reg_(reg) {}

    void moveRobot(RobotId id, Position dst);
    void startRobotWork(RobotId id, const std::string& kind);
    void stopRobot(RobotId id);
    void printRobots() const;

private:
    RobotRegistry* reg_;
};
