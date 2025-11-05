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

    // not in use yet
    void printRobots() const;
    // bootstrap the environment map with size and dirt spots
    void seedFrom(const BootstrapFeed& feed);
    // main control loop
    void run();

private:
    // command sending helpers  
    void sendMoveCmd(RobotId id, Position dst);
    void sendStartRobotWorkCmd(RobotId id, const std::string& kind);
    void sendStopRobotCmd(RobotId id);
    // event retrieval - called by the control unit to process robot reports
    void drainEvents();
    // event handling helpers
    void handleEvent(const Bus::EventVariant& event);
    void handleStatusEvent(const StatusEvent& event);
    void handleWorkCompletedEvent(const WorkCompletedEvent& event);

    // task processing helpers
    bool processVacuumQueue();
    bool processWasherQueue();
    // enqueue a CELL for vacuuming to the vacuumQueue_(the enqueue for washer is done internally after vacuum)
    bool enqueueVacuumTask(Position pos);
    // find the nearest idle robot of the given type to the target position
    std::shared_ptr<RobotBase> findNearestIdleRobot(RobotType type, Position target) const;

    // members - the core components
    RobotRegistry& reg_;
    EnvironmentMap& map_;
    Planner        planner_;
    Bus            bus_;

    // store inside pendingTasks_ map for tracking which job is still pending to be done
    struct PendingTask {
        std::string kind;
        Position    target;
    };

    // task queues and bookkeeping
    std::queue<Position> vacuumQueue_;
    std::queue<Position> washerQueue_;
    // to avoid duplicate entries of the same cell
    std::set<std::pair<int,int>> queuedForVacuum_;
    std::set<std::pair<int,int>> queuedForWasher_;
    // to know which task is pending for which robot
    std::unordered_map<RobotId, PendingTask> pendingTasks_;
};
