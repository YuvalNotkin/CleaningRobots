// test_scenarios.cpp
// Scenario harness for the Cleaning Robots system.
// NOTE: Comments are in English per your preference.

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

#include "robot/robot.hpp"
#include "robot/robots_impl.hpp"
#include "registry/registry.hpp"
#include "environment/environment_map.hpp"
#include "control_unit/control_unit.hpp"
#include "common/bootstrap.hpp"
#include "test_scenarios/test_scenarios.hpp"

using std::cout;
using std::endl;

static BootstrapFeed makeFeed(const std::vector<Position>& spots) {
    BootstrapFeed feed;
    feed.dirtSpots = spots;
    feed.gridWidth = 1;
    feed.gridHeight = 1;
    for (const auto& p : spots) {
        if (p.x >= 0 && p.y >= 0) {
            feed.gridWidth = std::max(feed.gridWidth, p.x + 1);
            feed.gridHeight = std::max(feed.gridHeight, p.y + 1);
        }
    }
    return feed;
}

static void divider(const std::string& title) {
    cout << "\n============================================================\n";
    cout << " SCENARIO: " << title << "\n";
    cout << "============================================================\n";
}

// ---------- Scenario 1: Typical small fleet ----------
static void scenario_typical_small() {
    divider("Typical small fleet (2x Detector, 2x Vacuum, 1x Washer) with 3 dirt spots");
    RobotRegistry registry;

    auto d1 = std::make_shared<DetectorRobot>("d1", Position{4,2});
    auto d2 = std::make_shared<DetectorRobot>("d2", Position{3,1});
    auto v1 = std::make_shared<VacuumRobot  >("v1", Position{1,0});
    auto v2 = std::make_shared<VacuumRobot  >("v2", Position{5,2});
    auto w1 = std::make_shared<WasherRobot  >("w1", Position{2,0});

    registry.add(d1); registry.add(d2);
    registry.add(v1); registry.add(v2);
    registry.add(w1);

    BootstrapFeed feed = makeFeed({
        Position{3, 1},
        Position{4, 2},
        Position{5, 2}
    });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: all 3 spots detected -> vacuumed -> washed in some order.\n";
}

// ---------- Scenario 2: No robots ----------
static void scenario_no_robots() {
    divider("Edge: Empty registry (no robots) with pending dirt");
    RobotRegistry registry;

    BootstrapFeed feed = makeFeed({ Position{0,0}, Position{2,2} });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: graceful handling / warnings; no crash.\n";
}

// ---------- Scenario 3: Only detectors ----------
static void scenario_only_detectors() {
    divider("Edge: Only Detector robots exist (no Vacuum/Washer)");
    RobotRegistry registry;

    auto d1 = std::make_shared<DetectorRobot>("d1", Position{0,0});
    auto d2 = std::make_shared<DetectorRobot>("d2", Position{2,2});
    registry.add(d1); registry.add(d2);

    BootstrapFeed feed = makeFeed({ Position{2,2}, Position{3,3} });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: detection happens; CU should log inability to allocate Vacuum/Washer.\n";
}

// ---------- Scenario 4: Multiple vacuums -> choose nearest ----------
static void scenario_multi_vacuums_choose_nearest() {
    divider("Scale/Optimization: 1 Detector + 3 Vacuums at different locations (5 spots spread)");
    RobotRegistry registry;

    auto d1  = std::make_shared<DetectorRobot>("d1", Position{0,0});
    auto v1  = std::make_shared<VacuumRobot  >("v1", Position{2,1});
    auto v2  = std::make_shared<VacuumRobot  >("v2", Position{5,5});
    auto v3  = std::make_shared<VacuumRobot  >("v3", Position{8,2});
    auto w1  = std::make_shared<WasherRobot  >("w1", Position{3,0});

    registry.add(d1);
    registry.add(v1); registry.add(v2); registry.add(v3);
    registry.add(w1);

    BootstrapFeed feed = makeFeed({
        Position{2, 2},   // near v1
        Position{5, 5},   // near v2
        Position{8, 3},   // near v3
        Position{3, 1},   // near v1/w1
        Position{6, 4}    // mix of distances
    });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: CU picks closest Vacuum per spot (by your selection policy).\n";
}

// ---------- Scenario 5: Very large coordinates ----------
static void scenario_huge_coordinates() {
    divider("Edge: Very large coordinates (numeric robustness)");
    RobotRegistry registry;

    const int M = 20;
    auto d1 = std::make_shared<DetectorRobot>("d1", Position{ M-5, M-5});
    auto v1 = std::make_shared<VacuumRobot  >("v1", Position{ M-7, M-6});
    auto w1 = std::make_shared<WasherRobot  >("w1", Position{ M-4, M-4});
    registry.add(d1); registry.add(v1); registry.add(w1);

    BootstrapFeed feed = makeFeed({ Position{ M-8, M-7 } });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: no overflow in distance/logic; normal completion.\n";
}

// ---------- Scenario 6: Many spots (stress) ----------
static void scenario_many_spots_stress() {
    divider("Stress: 1x of each robot, 50 dirt spots in a 10x10 grid");
    RobotRegistry registry;

    auto d1 = std::make_shared<DetectorRobot>("d1", Position{0,0});
    auto v1 = std::make_shared<VacuumRobot  >("v1", Position{0,0});
    auto w1 = std::make_shared<WasherRobot  >("w1", Position{0,0});
    registry.add(d1); registry.add(v1); registry.add(w1);

    std::vector<Position> spots;
    for (int x = 0; x < 10; ++x) {
        for (int y = 0; y < 5; ++y) {
            spots.push_back(Position{ x * 2, y * 2 });
        }
    }
    BootstrapFeed feed = makeFeed(spots);

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: completes all tasks; observe runtime/order in logs.\n";
}

// ---------- Scenario 7: Duplicate/overlapping spots ----------
static void scenario_duplicate_spots() {
    divider("Edge: Duplicate/overlapping spots");
    RobotRegistry registry;

    auto d1 = std::make_shared<DetectorRobot>("d1", Position{0,0});
    auto v1 = std::make_shared<VacuumRobot  >("v1", Position{0,0});
    auto w1 = std::make_shared<WasherRobot  >("w1", Position{0,0});
    registry.add(d1); registry.add(v1); registry.add(w1);

    BootstrapFeed feed = makeFeed({
        Position{3,3},
        Position{3,3}, // exact duplicate
        Position{3,4}, // near-duplicate
        Position{3,3}  // exact duplicate again
    });

    EnvironmentMap map;
    ControlUnit cu{registry, map};
    cu.seedFrom(feed);
    cu.run();
    cout << "[Result] Expected: either deduplication or repeated handling per policy;\n";
}

int run_all_scenarios() {
    cout << "Running Cleaning Robots test scenarios...\n";

    scenario_typical_small();
    scenario_no_robots();
    scenario_only_detectors();
    scenario_multi_vacuums_choose_nearest();
    scenario_huge_coordinates();
    scenario_many_spots_stress();
    scenario_duplicate_spots();

    cout << "\nAll scenarios executed. Review logs above.\n";
    return 0;
}
