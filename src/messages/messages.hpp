#pragma once
#include <string>
#include "common/types.hpp"   // RobotId, Position, RobotType, RobotState

// -------------------------
// Events (robot → control)
// -------------------------

// A detector reports that it found a new target location - not used currently.
struct DetectionEvent {
    RobotId   from{0};     
    Position  position{};  
};

// Any robot reports its current status (used for monitoring and updates).
struct StatusEvent {
    RobotId     from{0};              
    RobotType   type{RobotType::DETECTOR}; 
    RobotState  state{RobotState::IDLE};
    Position    position{};           
};

// A robot notifies that it has finished its work (vacuuming, washing, etc.).
struct WorkCompletedEvent {
    RobotId      from{0};      
    std::string  workKind;     // "DETECT" / "VACUUM" / "WASH"
    Position     position{};   
    bool         success{true}; // true if finished successfully
};

// -------------------------
// Commands (control → robot)
// -------------------------

// Order a robot to move somewhere.
struct MoveCommand {
    RobotId  to{0};       
    Position position{};  // destination
};

// Order a robot to start a specific kind of work.
struct StartWorkCommand {
    RobotId     to{0};    
    std::string kind;     // "DETECT" / "VACUUM" / "WASH"
};

// Order a robot to stop its current work.
struct StopCommand {
    RobotId to{0};        
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
