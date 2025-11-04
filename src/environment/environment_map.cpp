#include "environment/environment_map.hpp"

#include <iostream>

bool EnvironmentMap::initializeGrid(int width, int height, const std::vector<Position>& dirtSpots) {
    if (width <= 0 || height <= 0) {
        std::cerr << "[Map] grid dimensions must be positive (" << width << "x" << height << ")\n";
        return false;
    }

    width_ = width;
    height_ = height;
    grid_.assign(height_, std::vector<CellState>(width_, CellState::CLEAN));

    for (const auto& spot : dirtSpots) {
        if (!inBounds(spot)) {
            std::cerr << "[Map] dirt spot out of bounds at (" << spot.x << "," << spot.y << ")\n";
            grid_.clear();
            width_ = height_ = 0;
            return false;
        }
        grid_[spot.y][spot.x] = CellState::DIRTY;
    }
    return true;
}

bool EnvironmentMap::hasDirt(Position p) const {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    return grid_[p.y][p.x] == CellState::DIRTY;
}

bool EnvironmentMap::markVacuumed(Position p) {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    CellState& cell = grid_[p.y][p.x];
    if (cell != CellState::DIRTY) {
        return false;
    }
    cell = CellState::VACUUMED;
    return true;
}

bool EnvironmentMap::needsWash(Position p) const {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    return grid_[p.y][p.x] == CellState::VACUUMED;
}

bool EnvironmentMap::markWashed(Position p) {
    if (!inBounds(p) || grid_.empty()) {
        return false;
    }
    CellState& cell = grid_[p.y][p.x];
    if (cell != CellState::VACUUMED) {
        return false;
    }
    cell = CellState::CLEAN;
    return true;
}

bool EnvironmentMap::inBounds(Position p) const {
    return p.x >= 0 && p.y >= 0 && p.x < width_ && p.y < height_;
}
