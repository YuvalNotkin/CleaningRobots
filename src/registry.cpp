#include "include/registry.hpp"

void RobotRegistry::add(std::shared_ptr<IRobot> r) {
    robots_.push_back(std::move(r));
}

std::shared_ptr<IRobot> RobotRegistry::getById(RobotId id) const {
    for (auto& r : robots_)
        if (r->id() == id)
            return r;
    return nullptr;
}

const std::vector<std::shared_ptr<IRobot>>& RobotRegistry::all() const {
    return robots_;
}
