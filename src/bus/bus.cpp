#include "bus/bus.hpp"

#include "robot/robot.hpp"

// Initialize the bus with a reference to the robot registry, attach to all registered robots
Bus::Bus(RobotRegistry& registry) : registry_(registry) {
    auto robots = registry_.getAll();
    for (const auto& robot : robots) {
        if (robot) {
            robot->attachBus(this);
        }
    }
}

/////////// command broadcasting - called by the control unit to direct robot actions

void Bus::broadcast(MoveCommand cmd) {
    broadcastImpl(cmd);
}

void Bus::broadcast(StartWorkCommand cmd) {
    broadcastImpl(cmd);
}

void Bus::broadcast(StopCommand cmd) {
    broadcastImpl(cmd);
}

/////////// event publishing - called by robots to report happenings

void Bus::publish(DetectionEvent event) {
    publishImpl(event);
}

void Bus::publish(StatusEvent event) {
    publishImpl(event);
}

void Bus::publish(WorkCompletedEvent event) {
    publishImpl(event);
}


bool Bus::poll(EventVariant& out) {     // out is reference, fill it with next event only if any
    if (events_.empty()) {
        return false;
    }
    out = std::move(events_.front());
    events_.pop();
    return true;
}

template<typename Command>  
void Bus::broadcastImpl(Command& cmd) { // send command to all robots, let them decide if relevant
    auto robots = registry_.getAll();
    for (const auto& robot : robots) {
        if (robot) {
            robot->handle(cmd);
        }
    }
}

template<typename Event>
void Bus::publishImpl(Event& event) { // add event to the queue for later processing by the CU
    events_.push(event);
}

// Force template code generation for these types in this .cpp file
// (avoids duplicate instantiations across translation units)
template void Bus::broadcastImpl(MoveCommand& cmd);
template void Bus::broadcastImpl(StartWorkCommand& cmd);
template void Bus::broadcastImpl(StopCommand& cmd);

template void Bus::publishImpl(DetectionEvent& event);
template void Bus::publishImpl(StatusEvent& event);
template void Bus::publishImpl(WorkCompletedEvent& event);
