#pragma once
#include <memory>
#include <vector>

#include "robot.hpp"

class RobotRegistry {
public:
    bool initializeGrid(int width, int height, const std::vector<Position>& dirtSpots);

    bool add(std::shared_ptr<IRobot> r);
    std::shared_ptr<IRobot> getById(RobotId id) const;
    std::vector<std::shared_ptr<IRobot>> getByType(RobotType t) const;
    
    bool hasDirt(Position p) const;
    bool markVacuumed(Position p);
    bool needsWash(Position p) const;
    bool markWashed(Position p);
    bool inBounds(Position p) const;

    int width() const { return width_; }
    int height() const { return height_; }
    const std::vector<std::vector<int>>& grid() const { return grid_; }

private:
    std::vector<std::shared_ptr<IRobot>> robots_;
    int width_{0};
    int height_{0};
    std::vector<std::vector<int>> grid_;
};
