#pragma once

#include "robot/robot.hpp"

// Vacuum robot collects dirt once located.
class VacuumRobot final : public RobotBase {
public:
    VacuumRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::VACUUM, start) {}

    void moveTo(Position dst) override;
    void startWork(const std::string& kind) override;
    void stop() override;
};
