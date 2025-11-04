#pragma once
#include <queue>
#include <set>
#include <unordered_map>
#include <utility>
#include <string>
#include "registry/registry.hpp"
#include "environment/environment_map.hpp"
#include "planner/planner.hpp"
#include "common/bootstrap.hpp"
#include "bus/bus.hpp"

class ControlUnit {
public:
    ControlUnit(RobotRegistry& reg, EnvironmentMap& map): reg_(reg), map_(map), bus_(reg) {}

    void printRobots() const;
    void seedFrom(const BootstrapFeed& feed);
    void run();

private:
    void sendMoveCmd(RobotId id, Position dst);
    void sendStartRobotWorkCmd(RobotId id, const std::string& kind);
    void sendStopRobotCmd(RobotId id);
    void drainEvents();
    void handleEvent(const Bus::EventVariant& event);
    void handleStatusEvent(const StatusEvent& event);
    void handleWorkCompletedEvent(const WorkCompletedEvent& event);

    bool processVacuumQueue();
    bool processWasherQueue();
    bool enqueueVacuumTask(Position pos);
    std::shared_ptr<RobotBase> findNearestIdleRobot(RobotType type, Position target) const;

    RobotRegistry& reg_;
    EnvironmentMap& map_;
    Planner        planner_;
    Bus            bus_;

    struct PendingTask {
        std::string kind;
        Position    target;
    };

    std::queue<Position> vacuumQueue_;
    std::queue<Position> washerQueue_;
    std::set<std::pair<int,int>> queuedForVacuum_;
    std::set<std::pair<int,int>> queuedForWasher_;
    std::unordered_map<RobotId, PendingTask> pendingTasks_;
};
