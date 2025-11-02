#pragma once
#include <string>
#include "types.hpp"   // RobotId, Position, RobotType, RobotState

// -------------------------
// Events (robot → control)
// -------------------------

// A detector reports that it found a new target location.
struct DetectionEvent {
    RobotId   from;        // detector robot ID
    Position  location;    // where the detection happened
};

// Any robot reports its current status (used for monitoring and updates).
struct StatusEvent {
    RobotId     from;      // sender robot ID
    RobotType   type;      // sender type
    RobotState  state;     // IDLE / MOVING / WORKING / ERROR
    Position    position;  // current position
};

// A robot notifies that it has finished its work (vacuuming, washing, etc.).
struct WorkCompletedEvent {
    RobotId      from;         // sender robot ID
    std::string  workKind;     // "DETECT" / "VACUUM" / "WASH"
    Position     where;        // location of the completed work
    bool         success{true}; // true if finished successfully
};

// -------------------------
// Commands (control → robot)
// -------------------------

// Order a robot to move somewhere.
struct MoveCommand {
    RobotId  to;          // target robot ID
    Position dst;         // destination
};

// Order a robot to start a specific kind of work.
struct StartWorkCommand {
    RobotId     to;       // target robot ID
    std::string kind;     // "DETECT" / "VACUUM" / "WASH"
};

// Order a robot to stop its current work.
struct StopCommand {
    RobotId to;           // target robot ID
};

// Optional time broadcast — kept for future extension but unused now.
struct TickCommand {
    unsigned long long now{0};
};

// -------------------------
// Simple string helpers
// -------------------------
std::string toString(const DetectionEvent&);
std::string toString(const StatusEvent&);
std::string toString(const WorkCompletedEvent&);
std::string toString(const MoveCommand&);
std::string toString(const StartWorkCommand&);
std::string toString(const StopCommand&);
std::string toString(const TickCommand&);
