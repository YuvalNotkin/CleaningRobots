#pragma once
#include <cstddef>
#include <vector>

#include "robot/robot.hpp"    // Position, RobotType

class Planner {
public:
    void configureGrid(int width, int height);

    bool isConfigured() const { return width_ > 0 && height_ > 0; }
    int width() const { return width_; }
    int height() const { return height_; }

    std::vector<std::vector<Position>> buildScanPlans(std::size_t detectorCount) const;

private:
    using PatternGenerator = std::vector<Position> (Planner::*)() const;

    std::vector<Position> rowWisePattern() const;
    std::vector<Position> columnWisePattern() const;

    int width_{0};
    int height_{0};
};
