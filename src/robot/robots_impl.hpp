#pragma once
#include "robot/robot.hpp"

// Concrete robot specializations by role.

// Detector robot scans rooms for dirt.
class DetectorRobot final : public RobotBase {
public:
    DetectorRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::DETECTOR, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

// Vacuum robot collects dirt once located.
class VacuumRobot final : public RobotBase {
public:
    VacuumRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::VACUUM, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

// Washer robot finishes the cleanup pass.
class WasherRobot final : public RobotBase {
public:
    WasherRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::WASHER, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};
