#include "planner/planner.hpp"

#include <algorithm>
#include <cstddef>

void Planner::configureGrid(int width, int height) {
    width_ = width;
    height_ = height;
}

std::vector<std::vector<Position>> Planner::buildScanPlans(std::size_t detectorCount) const {
    if (!isConfigured() || detectorCount == 0) {
        return {};
    }

    std::vector<PatternGenerator> generators = {
        &Planner::rowWisePattern,
        &Planner::columnWisePattern
    };

    std::vector<std::vector<Position>> plans;
    plans.reserve(detectorCount);

    for (std::size_t idx = 0; idx < detectorCount; ++idx) {
        PatternGenerator generator = generators[idx % generators.size()];
        std::vector<Position> pattern = (this->*generator)();
        plans.push_back(std::move(pattern));
    }

    return plans;
}

std::vector<Position> Planner::rowWisePattern() const {
    std::vector<Position> path;
    path.reserve(static_cast<std::size_t>(width_) * height_);
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            path.push_back(Position{x, y});
        }
    }
    return path;
}

std::vector<Position> Planner::columnWisePattern() const {
    std::vector<Position> path;
    path.reserve(static_cast<std::size_t>(width_) * height_);
    for (int x = 0; x < width_; ++x) {
        for (int y = 0; y < height_; ++y) {
            path.push_back(Position{x, y});
        }
    }
    return path;
}
