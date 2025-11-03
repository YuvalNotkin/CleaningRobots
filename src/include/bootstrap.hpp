#pragma once
#include <vector>
#include "robot.hpp" 

struct BootstrapFeed {
    int gridWidth{0};
    int gridHeight{0};
    std::vector<Position> dirtSpots;
};
    
