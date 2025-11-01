#include <memory>
#include "include/types.hpp"
#include "include/registry.hpp"
#include "include/control_unit.hpp"
#include "include/robots_impl.hpp"

int main() {
    RobotRegistry registry;

    auto det = std::make_shared<DetectorRobot>(Position{0,0});
    auto vac1 = std::make_shared<VacuumRobot>(Position{0,0});
    auto vac2 = std::make_shared<VacuumRobot>(Position{2,1});
    auto was  = std::make_shared<WasherRobot>(Position{0,0});

    registry.add(det);
    registry.add(vac1);
    registry.add(vac2);
    registry.add(was);

    ControlUnit cu(&registry);

    cu.moveRobot(2, {5,3});
    cu.startRobotWork(2, "VACUUM");   
    cu.moveRobot(3, {5,3});            
    cu.startRobotWork(3, "WASH");      

    return 0;
}
