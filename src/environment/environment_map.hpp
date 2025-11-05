#pragma once

#include <vector>

#include "robot/robot.hpp"

enum class CellState { CLEAN = 0, DIRTY = 1, VACUUMED = 2 };

// EnvironmentMap keeps the grid definition and dirt lifecycle state.
class EnvironmentMap {
public:
    bool initializeGrid(int width, int height, const std::vector<Position>& dirtSpots);

    // Helpers for dirt lifecycle
    bool hasDirt(Position p) const;
    bool markVacuumed(Position p);
    bool needsWash(Position p) const;
    bool markWashed(Position p);
    bool inBounds(Position p) const;

    // Getters
    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<std::vector<CellState>>& grid() const { return grid_; }

private:
    int width_{0};
    int height_{0};
    std::vector<std::vector<CellState>> grid_;
};
