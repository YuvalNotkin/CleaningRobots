#pragma once
#include <queue>
#include <variant>
#include <vector>

#include "messages/messages.hpp"
#include "registry/registry.hpp"

// In-process message bus that routes commands to robots and buffers their events.
class Bus {
public:
    using EventVariant = std::variant<DetectionEvent, StatusEvent, WorkCompletedEvent>;

    Bus(RobotRegistry& registry);

    // command broadcasting - called by the control unit to direct robot actions
    void broadcast(MoveCommand cmd);        
    void broadcast(StartWorkCommand cmd);   
    void broadcast(StopCommand cmd);        

    // event publishing - called by robots to report happenings
    void publish(DetectionEvent event);     
    void publish(StatusEvent event);
    void publish(WorkCompletedEvent event);

    // event retrieval - called by the control unit to process robot reports
    bool poll(EventVariant& out);

private:
    template<typename Command>
    // fan-out helper that forwards the command to all registered robots
    void broadcastImpl(Command& cmd);

    template<typename Event>
    // enqueue the event for later retrieval via poll()
    void publishImpl(Event& event);

    RobotRegistry& registry_;
    std::queue<EventVariant> events_;
};
