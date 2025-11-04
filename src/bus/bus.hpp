#pragma once
#include <queue>
#include <variant>
#include <vector>

#include "messages/messages.hpp"
#include "registry/registry.hpp"

class Bus {
public:
    using EventVariant = std::variant<DetectionEvent, StatusEvent, WorkCompletedEvent>;

    Bus(RobotRegistry& registry);

    void broadcast(MoveCommand cmd);
    void broadcast(StartWorkCommand cmd);
    void broadcast(StopCommand cmd);

    void publish(DetectionEvent event);
    void publish(StatusEvent event);
    void publish(WorkCompletedEvent event);

    bool poll(EventVariant& out);

private:
    template<typename Command>
    void broadcastImpl(Command& cmd);

    template<typename Event>
    void publishImpl(Event& event);

    RobotRegistry& registry_;
    std::queue<EventVariant> events_;
};
