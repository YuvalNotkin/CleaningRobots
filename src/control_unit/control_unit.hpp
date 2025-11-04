#pragma once
#include <queue>
#include <set>
#include <utility>
#include <string>
#include "registry/registry.hpp"
#include "environment/environment_map.hpp"
#include "planner/planner.hpp"
#include "common/bootstrap.hpp"

class ControlUnit {
public:
    ControlUnit(RobotRegistry& reg, EnvironmentMap& map)
        : reg_(reg), map_(map) {}

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
    std::shared_ptr<RobotBase> findNearestIdleRobot(RobotType type, Position target) const;

    RobotRegistry& reg_;
    EnvironmentMap& map_;
    Planner        planner_;
    std::queue<Position> vacuumQueue_;
    std::queue<Position> washerQueue_;
    std::set<std::pair<int,int>> queuedForVacuum_;
    std::set<std::pair<int,int>> queuedForWasher_;
};
