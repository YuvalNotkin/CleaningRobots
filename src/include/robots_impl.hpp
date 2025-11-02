#pragma once
#include "robot.hpp"

class DetectorRobot : public RobotBase {
public:
    DetectorRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::DETECTOR, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

class VacuumRobot : public RobotBase {
public:
    VacuumRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::VACUUM, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

class WasherRobot : public RobotBase {
public:
    WasherRobot(RobotName name, Position start = {})
        : RobotBase(std::move(name), RobotType::WASHER, start) {}
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};
