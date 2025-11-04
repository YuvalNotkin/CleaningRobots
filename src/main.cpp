#include <memory>
#include <iostream>
#include "robot/robot.hpp"
#include "robot/robots_impl.hpp"  // your concrete DetectorRobot, VacuumRobot, WasherRobot
#include "registry/registry.hpp"
#include "control_unit/control_unit.hpp"
#include "common/bootstrap.hpp"
#include "test_scenarios/test_scenarios.hpp"

int main() {
    run_all_scenarios();
    return 0;
}
