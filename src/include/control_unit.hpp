#pragma once
#include <string>
#include "registry.hpp"
#include "planner.hpp"
#include "bootstrap.hpp"

// Orchestrator over Registry + Planner
class ControlUnit {
public:
    explicit ControlUnit(RobotRegistry& reg) : reg_(reg) {}

    // --- Simple helpers (optional, used by tests/manual control) ---
    void moveRobot(RobotId id, Position dst);
    void startRobotWork(RobotId id, const std::string& kind);
    void stopRobot(RobotId id);
    void printRobots() const;

    // --- Simulation flow (what main() uses now) ---
    void seedFrom(const BootstrapFeed& feed); // push DETECT jobs to planner
    void runUntilIdle();                      // while(planner.hasWork()) step()

private:
    RobotRegistry& reg_;
    Planner        planner_;
};
