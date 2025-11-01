#pragma once
#include <memory>
#include <vector>
#include "robot.hpp"


class RobotRegistry {
public:
    void add(std::shared_ptr<IRobot> r);
    std::shared_ptr<IRobot> getById(RobotId id) const;
    const std::vector<std::shared_ptr<IRobot>>& all() const;
private:
    std::vector<std::shared_ptr<IRobot>> robots_;
};
