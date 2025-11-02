#include "include/registry.hpp"

#include <algorithm>
#include <limits>
#include <utility>
#include <mutex>

namespace {
    constexpr int square(int value) { return value * value; }
}

std::size_t RobotRegistry::typeIndex(RobotType t) {
    return static_cast<std::size_t>(t);
}

int RobotRegistry::cellCoord(int value) {
    if (value >= 0) {
        return value / kCellSize;
    }
    return (value - (kCellSize - 1)) / kCellSize;
}

RobotRegistry::Cell RobotRegistry::cellFromPosition(Position p) const {
    return {cellCoord(p.x), cellCoord(p.y)};
}

bool RobotRegistry::add(std::shared_ptr<IRobot> r) {
    if (!r) {
        return false;
    }

    std::unique_lock lock(mutex_);
    RobotId id = r->id();

    auto [it, inserted] = byId_.emplace(id, std::move(r));
    if (!inserted) {
        return false;
    }

    const RobotType type = it->second->type();
    const Position pos = it->second->position();

    byType_[typeIndex(type)].insert(id);
    posById_[id] = pos;
    grid_[cellFromPosition(pos)].insert(id);

    return true;
}

bool RobotRegistry::remove(RobotId id) {
    std::unique_lock lock(mutex_);

    auto it = byId_.find(id);
    if (it == byId_.end()) {
        return false;
    }

    const RobotType type = it->second->type();
    byType_[typeIndex(type)].erase(id);

    if (auto posIt = posById_.find(id); posIt != posById_.end()) {
        const Cell oldCell = cellFromPosition(posIt->second);
        if (auto gridIt = grid_.find(oldCell); gridIt != grid_.end()) {
            gridIt->second.erase(id);
            if (gridIt->second.empty()) {
                grid_.erase(gridIt);
            }
        }
        posById_.erase(posIt);
    }

    byId_.erase(it);
    return true;
}

bool RobotRegistry::updatePosition(RobotId id, Position p) {
    std::unique_lock lock(mutex_);

    auto robotIt = byId_.find(id);
    if (robotIt == byId_.end()) {
        return false;
    }

    const Cell newCell = cellFromPosition(p);
    Cell oldCell{};
    bool hadOldCell = false;

    if (auto posIt = posById_.find(id); posIt != posById_.end()) {
        oldCell = cellFromPosition(posIt->second);
        hadOldCell = true;
        posIt->second = p;
    } else {
        posById_.emplace(id, p);
    }

    if (hadOldCell) {
        if (auto gridIt = grid_.find(oldCell); gridIt != grid_.end()) {
            gridIt->second.erase(id);
            if (gridIt->second.empty()) {
                grid_.erase(gridIt);
            }
        }
    }

    grid_[newCell].insert(id);
    return true;
}

std::shared_ptr<IRobot> RobotRegistry::getById(RobotId id) const {
    std::shared_lock lock(mutex_);

    if (auto it = byId_.find(id); it != byId_.end()) {
        return it->second;
    }
    return nullptr;
}

std::vector<std::shared_ptr<IRobot>> RobotRegistry::getByType(RobotType t) const {
    std::shared_lock lock(mutex_);

    const auto idx = typeIndex(t);
    if (idx >= NUM_TYPES) {
        return {};
    }

    std::vector<std::shared_ptr<IRobot>> result;
    result.reserve(byType_[idx].size());
    for (RobotId id : byType_[idx]) {
        if (auto it = byId_.find(id); it != byId_.end()) {
            result.push_back(it->second);
        }
    }
    return result;
}

std::shared_ptr<IRobot> RobotRegistry::findNearest(RobotType t, Position target, int maxRadiusCells) const {
    std::shared_lock lock(mutex_);

    const auto idx = typeIndex(t);
    if (idx >= NUM_TYPES || byType_[idx].empty() || maxRadiusCells < 0) {
        return nullptr;
    }

    const Cell center = cellFromPosition(target);
    std::shared_ptr<IRobot> best;
    int bestDistSq = std::numeric_limits<int>::max();

    for (int radius = 0; radius <= maxRadiusCells; ++radius) {
        bool foundInRadius = false;
        for (int cx = center.x - radius; cx <= center.x + radius; ++cx) {
            for (int cy = center.y - radius; cy <= center.y + radius; ++cy) {
                const int dxCell = cx - center.x;
                const int dyCell = cy - center.y;
                const int absDxCell = dxCell < 0 ? -dxCell : dxCell;
                const int absDyCell = dyCell < 0 ? -dyCell : dyCell;
                if (std::max(absDxCell, absDyCell) != radius) {
                    continue;
                }

                const Cell cell{cx, cy};
                auto gridIt = grid_.find(cell);
                if (gridIt == grid_.end()) {
                    continue;
                }

                for (RobotId id : gridIt->second) {
                    auto robotIt = byId_.find(id);
                    if (robotIt == byId_.end() || robotIt->second->type() != t) {
                        continue;
                    }
                    // Skip non-idle robots; only IDLE robots are available for assignment.
                    if (robotIt->second->state() != RobotState::IDLE) {
                        continue;
                    }
                    auto posIt = posById_.find(id);
                    if (posIt == posById_.end()) {
                        continue;
                    }

                    const int dx = posIt->second.x - target.x;
                    const int dy = posIt->second.y - target.y;
                    const int distSq = square(dx) + square(dy);
                    if (distSq < bestDistSq) {
                        bestDistSq = distSq;
                        best = robotIt->second;
                        foundInRadius = true;
                    }
                }
            }
        }

        if (foundInRadius && best) {
            break;
        }
    }

    // If nothing suitable was found within maxRadiusCells, perform a global fallback
    // across all robots of the requested type and pick the closest available (IDLE) one.
    if (!best) {
        for (RobotId id : byType_[idx]) {
            auto robotIt = byId_.find(id);
            if (robotIt == byId_.end() || robotIt->second->type() != t) {
                continue;
            }
            if (robotIt->second->state() != RobotState::IDLE) {
                continue;
            }
            auto posIt = posById_.find(id);
            if (posIt == posById_.end()) {
                continue;
            }

            const int dx = posIt->second.x - target.x;
            const int dy = posIt->second.y - target.y;
            const int distSq = square(dx) + square(dy);
            if (distSq < bestDistSq) {
                bestDistSq = distSq;
                best = robotIt->second;
            }
        }
    }

    return best;
}
