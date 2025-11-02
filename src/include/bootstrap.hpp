#pragma once
#include <vector>
#include "robot.hpp"   // Position

// Plain data holder created and initialized only in main().
struct BootstrapFeed {
    std::vector<Position> dirtSpots; // initial targets for the simulation
};
    