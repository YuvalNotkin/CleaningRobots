#pragma once
#include <utility>
#include "common/types.hpp"
#include "common/ids.hpp"
#include "messages/messages.hpp"

class Bus;

// Abstract robot interface: concrete robots derive from this to interact with the bus.
class RobotBase {
public:
    virtual ~RobotBase() = default;

    // getters
    RobotId id() const { return id_; }          // unique runtime identifier
    RobotName name() const { return name_; }    // human-readable label
    RobotType type() const { return type_; }    // logical role (detector, vacuum, washer)
    RobotState state() const { return state_; } // coarse-grained activity state
    Position position() const { return pos_; }  // last known grid location

    // API actions - to be implemented in the concrete robot classes, later to be extended with realistic logic
    virtual void moveTo(Position dst) = 0;
    virtual void startWork(const std::string& kind) = 0;
    virtual void stop() = 0;

    // bus interaction - called by the Bus when broadcasting commands and activating the actions above
    void attachBus(Bus* bus);
    void handle(const MoveCommand& cmd);
    void handle(const StartWorkCommand& cmd);
    void handle(const StopCommand& cmd);

protected:
    RobotBase(RobotName name, RobotType type, Position start = {}) : id_(IdGenerator::next()), name_(std::move(name)), type_(type), pos_(start) {}

    // event publishing helpers - to be called by derived classes when relevant events occur
    void publishStatus();
    void publishWorkCompleted(const std::string& kind, bool success);

    RobotId id_;
    RobotName name_;
    RobotType type_;
    RobotState state_{RobotState::IDLE};
    Position pos_{};
    Bus* bus_{nullptr};
};
