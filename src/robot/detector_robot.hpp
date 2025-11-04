#pragma once

#include "robot/robot.hpp"

// Detector robot scans rooms for dirt.
class DetectorRobot final : public RobotBase {
public:
    DetectorRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::DETECTOR, start) {}

    void moveTo(Position dst) override;
    void startWork(const std::string& kind) override;
    void stop() override;
};
