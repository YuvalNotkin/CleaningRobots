#pragma once
#include <array>
#include <cstddef>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "robot.hpp"

class RobotRegistry {
public:
    bool add(std::shared_ptr<IRobot> r);
    bool remove(RobotId id);
    bool updatePosition(RobotId id, Position p);

    std::shared_ptr<IRobot> getById(RobotId id) const;
    std::vector<std::shared_ptr<IRobot>> getByType(RobotType t) const;
    std::shared_ptr<IRobot> findNearest(RobotType t, Position target, int maxRadiusCells = 3) const;

private:
    struct Cell {
        int x{0};
        int y{0};

        bool operator==(const Cell& other) const noexcept {
            return x == other.x && y == other.y;
        }
    };

    struct CellHash {
        std::size_t operator()(const Cell& c) const noexcept {
            std::size_t hx = std::hash<int>{}(c.x);
            std::size_t hy = std::hash<int>{}(c.y);
            return hx ^ (hy + 0x9e3779b9 + (hx << 6) + (hx >> 2));
        }
    };

    static constexpr std::size_t NUM_TYPES = 3;
    static constexpr int kCellSize = 10;

    static std::size_t typeIndex(RobotType t);
    static int cellCoord(int value);
    Cell cellFromPosition(Position p) const;

    mutable std::shared_mutex mutex_;  // protects all registry data

    std::unordered_map<RobotId, std::shared_ptr<IRobot>> byId_;  // robots by identifier
    std::array<std::unordered_set<RobotId>, NUM_TYPES> byType_;  // robots grouped by type
    std::unordered_map<RobotId, Position> posById_;              // latest known robot positions
    std::unordered_map<Cell, std::unordered_set<RobotId>, CellHash> grid_;  // spatial index cells
};
