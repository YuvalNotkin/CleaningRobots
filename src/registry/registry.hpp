#pragma once
#include <memory>
#include <unordered_map>
#include <vector>

#include "robot/robot.hpp"

class RobotRegistry {
public:
    bool add(std::shared_ptr<RobotBase> r);
    std::shared_ptr<RobotBase> getById(RobotId id) const;
    std::vector<std::shared_ptr<RobotBase>> getByType(RobotType t) const;
    std::vector<std::shared_ptr<RobotBase>> getAll() const;

private:
    std::unordered_map<RobotId, std::shared_ptr<RobotBase>> robots_by_id_;
    std::unordered_map<RobotType, std::vector<std::shared_ptr<RobotBase>>> robots_by_type_;
};
