#include "bus/bus.hpp"

#include "robot/robot.hpp"

Bus::Bus(RobotRegistry& registry)
    : registry_(registry) {
    auto robots = registry_.getAll();
    for (const auto& robot : robots) {
        if (robot) {
            robot->attachBus(this);
        }
    }
}

void Bus::broadcast(MoveCommand cmd) {
    broadcastImpl(cmd);
}

void Bus::broadcast(StartWorkCommand cmd) {
    broadcastImpl(cmd);
}

void Bus::broadcast(StopCommand cmd) {
    broadcastImpl(cmd);
}

void Bus::publish(DetectionEvent event) {
    publishImpl(event);
}

void Bus::publish(StatusEvent event) {
    publishImpl(event);
}

void Bus::publish(WorkCompletedEvent event) {
    publishImpl(event);
}

bool Bus::poll(EventVariant& out) {
    if (events_.empty()) {
        return false;
    }
    out = std::move(events_.front());
    events_.pop();
    return true;
}

template<typename Command>
void Bus::broadcastImpl(Command& cmd) {
    auto robots = registry_.getAll();
    for (const auto& robot : robots) {
        if (robot) {
            robot->handle(cmd);
        }
    }
}

template<typename Event>
void Bus::publishImpl(Event& event) {
    events_.push(event);
}

// Explicit template instantiations to keep definitions in this TU.
template void Bus::broadcastImpl(MoveCommand& cmd);
template void Bus::broadcastImpl(StartWorkCommand& cmd);
template void Bus::broadcastImpl(StopCommand& cmd);

template void Bus::publishImpl(DetectionEvent& event);
template void Bus::publishImpl(StatusEvent& event);
template void Bus::publishImpl(WorkCompletedEvent& event);
