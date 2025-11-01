#pragma once
#include "robot.hpp"

class DetectorRobot : public RobotBase {
public:
    using RobotBase::RobotBase;
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

class VacuumRobot : public RobotBase {
public:
    using RobotBase::RobotBase;
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};

class WasherRobot : public RobotBase {
public:
    using RobotBase::RobotBase;
    void moveTo(Position) override;
    void startWork(const std::string&) override;
    void stop() override;
};
