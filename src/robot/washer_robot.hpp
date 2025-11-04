#pragma once

#include "robot/robot.hpp"

// Washer robot finishes the cleanup pass.
class WasherRobot final : public RobotBase {
public:
    WasherRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::WASHER, start) {}

    void moveTo(Position dst) override;
    void startWork(const std::string& kind) override;
    void stop() override;
};
