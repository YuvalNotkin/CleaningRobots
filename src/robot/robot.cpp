#include "robot/robot.hpp"

#include "bus/bus.hpp"

void RobotBase::attachBus(Bus* bus) {
    bus_ = bus;
}

void RobotBase::handle(const MoveCommand& cmd) {
    if (cmd.to != id_) {
        return;
    }
    moveTo(cmd.dst);
}

void RobotBase::handle(const StartWorkCommand& cmd) {
    if (cmd.to != id_) {
        return;
    }
    startWork(cmd.kind);
}

void RobotBase::handle(const StopCommand& cmd) {
    if (cmd.to != id_) {
        return;
    }
    stop();
    publishStatus();
}

void RobotBase::publishStatus() {
    if (!bus_) {
        return;
    }
    StatusEvent event;
    event.from = id_;
    event.type = type_;
    event.state = state_;
    event.position = pos_;
    bus_->publish(std::move(event));
}

void RobotBase::publishWorkCompleted(const std::string& kind, bool success) {
    if (!bus_) {
        return;
    }
    WorkCompletedEvent event;
    event.from = id_;
    event.workKind = kind;
    event.where = pos_;
    event.success = success;
    bus_->publish(std::move(event));
    publishStatus();
}
