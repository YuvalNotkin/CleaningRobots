#include "registry/registry.hpp"

bool RobotRegistry::add(std::shared_ptr<RobotBase> r) {
    if (!r) {
        return false;
    }
    const RobotId id = r->id();
    const RobotType type = r->type();

    auto [it, inserted] = robots_by_id_.emplace(id, std::move(r));
    if (!inserted) {
        return false;
    }

    robots_by_type_[type].push_back(it->second);
    return true;
}

std::shared_ptr<RobotBase> RobotRegistry::getById(RobotId id) const {
    auto it = robots_by_id_.find(id);
    if (it == robots_by_id_.end()) {
        return nullptr;
    }
    return it->second;
}

std::vector<std::shared_ptr<RobotBase>> RobotRegistry::getByType(RobotType t) const {
    auto it = robots_by_type_.find(t);
    if (it == robots_by_type_.end()) {
        return {};
    }

    return it->second;
}

std::vector<std::shared_ptr<RobotBase>> RobotRegistry::getAll() const {
    std::vector<std::shared_ptr<RobotBase>> result;
    result.reserve(robots_by_id_.size());
    for (const auto& [id, robot] : robots_by_id_) {
        (void)id;
        result.push_back(robot);
    }
    return result;
}
