#pragma once
#include <queue>
#include <set>
#include <utility>
#include <string>
#include "registry.hpp"
#include "planner.hpp"
#include "bootstrap.hpp"

class ControlUnit {
public:
    explicit ControlUnit(RobotRegistry& reg) : reg_(reg) {}

    void moveRobot(RobotId id, Position dst);
    void startRobotWork(RobotId id, const std::string& kind);
    void stopRobot(RobotId id);
    void printRobots() const;
    void seedFrom(const BootstrapFeed& feed);
    void run();

private:
    bool processVacuumQueue();
    bool processWasherQueue();
    bool enqueueVacuumTask(Position pos);
    std::shared_ptr<IRobot> findNearestIdleRobot(RobotType type, Position target) const;

    RobotRegistry& reg_;
    Planner        planner_;
    std::queue<Position> vacuumQueue_;
    std::queue<Position> washerQueue_;
    std::set<std::pair<int,int>> queuedForVacuum_;
    std::set<std::pair<int,int>> queuedForWasher_;
};
