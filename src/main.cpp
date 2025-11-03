#include <memory>
#include <iostream>
#include "include/robot.hpp"
#include "include/robots_impl.hpp"  // your concrete DetectorRobot, VacuumRobot, WasherRobot
#include "include/registry.hpp"
#include "include/control_unit.hpp"
#include "include/bootstrap.hpp"
#include "include/test_scenarios.hpp"

int main() {
    run_all_scenarios();
    return 0;
}
