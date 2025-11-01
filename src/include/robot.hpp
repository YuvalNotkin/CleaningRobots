#pragma once
#include "types.hpp"
#include "ids.hpp"

struct IRobot {
    virtual ~IRobot() = default;
    virtual RobotId    id() const = 0;
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
    RobotType type_;
    RobotState state_{RobotState::IDLE};
    Position pos_{};

explicit RobotBase(RobotType type, Position start = {})
    : id_(IdGenerator::next()), type_(type), pos_(start) {}

public:
    
    RobotId id() const override { return id_; }
    RobotType type() const override { return type_; }
    RobotState state() const override { return state_; }
    Position position() const override { return pos_; }
};
