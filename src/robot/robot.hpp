#pragma once
#include <utility>
#include "common/types.hpp"
#include "common/ids.hpp"
#include "messages/messages.hpp"

class Bus;

class RobotBase {
public:
    virtual ~RobotBase() = default;

    RobotId id() const { return id_; }
    RobotName name() const { return name_; }
    RobotType type() const { return type_; }
    RobotState state() const { return state_; }
    Position position() const { return pos_; }

    virtual void moveTo(Position dst) = 0;
    virtual void startWork(const std::string& kind) = 0;
    virtual void stop() = 0;

    void attachBus(Bus* bus);
    void handle(const MoveCommand& cmd);
    void handle(const StartWorkCommand& cmd);
    void handle(const StopCommand& cmd);

protected:
    RobotBase(RobotName name, RobotType type, Position start = {})
        : id_(IdGenerator::next()), name_(std::move(name)), type_(type), pos_(start) {}

    void publishStatus();
    void publishWorkCompleted(const std::string& kind, bool success);

    RobotId id_;
    RobotName name_;
    RobotType type_;
    RobotState state_{RobotState::IDLE};
    Position pos_{};
    Bus* bus_{nullptr};
};
