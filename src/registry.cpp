#include "include/registry.hpp"
#include <iostream>

bool RobotRegistry::initializeGrid(int width, int height, const std::vector<Position>& dirtSpots) {
    if (!(width > 0) || !(height > 0)) {
        std::cerr << "[Registry] grid dimensions must be positive (" << width << "x" << height << ")\n";
        return false;
    }

    width_ = width;
    height_ = height;
    grid_.assign(height_, std::vector<int>(width_, 0));

    for (const auto& spot : dirtSpots) {
        if (!inBounds(spot)) {
            std::cerr << "[Registry] dirt spot out of bounds at (" << spot.x << "," << spot.y << ")\n";
            grid_.clear();
            width_ = height_ = 0;
            return false;
        }
        grid_[spot.y][spot.x] = 1; // mark cell as containing dirt
    }
    return true;
}

bool RobotRegistry::add(std::shared_ptr<IRobot> r) {
    if (!r) {
        return false;
    }
    robots_.push_back(std::move(r));
    return true;
}

std::shared_ptr<IRobot> RobotRegistry::getById(RobotId id) const {
    for (const auto& robot : robots_) {
        if (robot && robot->id() == id) {
            return robot;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<IRobot>> RobotRegistry::getByType(RobotType t) const {
    std::vector<std::shared_ptr<IRobot>> result;
    for (const auto& robot : robots_) {
        if (robot && robot->type() == t) {
            result.push_back(robot);
        }
    }
    return result;
}

bool RobotRegistry::hasDirt(Position p) const {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    return grid_[p.y][p.x] == 1;
}

bool RobotRegistry::inBounds(Position p) const {
    return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_;
}

bool RobotRegistry::markVacuumed(Position p) {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    int& cell = grid_[p.y][p.x];
    if (cell != 1) {
        return false;
    }
    cell = 2;
    return true;
}

bool RobotRegistry::needsWash(Position p) const {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    return grid_[p.y][p.x] == 2;
}

bool RobotRegistry::markWashed(Position p) {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    int& cell = grid_[p.y][p.x];
    if (cell != 2) {
        return false;
    }
    cell = 0;
    return true;
}
