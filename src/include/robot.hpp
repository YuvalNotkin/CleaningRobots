#pragma once
#include <utility>
#include "types.hpp"
#include "ids.hpp"

struct IRobot {
    virtual ~IRobot() = default;
    virtual RobotId    id() const = 0;
    virtual RobotName  name() const = 0;
    virtual RobotType  type() const = 0;
    virtual RobotState state() const = 0;
    virtual Position   position() const = 0;

    virtual void moveTo(Position dst) = 0;
    virtual void startWork(const std::string& kind) = 0;
    virtual void stop() = 0;
};

class RobotBase : public IRobot {
protected:
    RobotId id_;
    RobotName name_;
    RobotType type_;
    RobotState state_{RobotState::IDLE};
    Position pos_{};

public:
    RobotBase(RobotName name, RobotType type, Position start = {})
        : id_(IdGenerator::next()), name_(std::move(name)), type_(type), pos_(start) {}

    RobotId id() const override { return id_; }
    RobotName name() const override { return name_; }
    RobotType type() const override { return type_; }
    RobotState state() const override { return state_; }
    Position position() const override { return pos_; }
};
