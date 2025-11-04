#include "messages/messages.hpp"
#include <sstream>

// Helper for formatting Position
static std::string posStr(Position p) {
    std::ostringstream os;
    os << "(" << p.x << "," << p.y << ")";
    return os.str();
}

// --------- Events ---------

std::string toString(const DetectionEvent& e) {
    std::ostringstream os;
    os << "[DetectionEvent] from=" << e.from
       << " loc=" << posStr(e.location);
    return os.str();
}

std::string toString(const StatusEvent& e) {
    std::ostringstream os;
    os << "[StatusEvent] from=" << e.from
       << " type=" << static_cast<int>(e.type)
       << " state=" << static_cast<int>(e.state)
       << " pos=" << posStr(e.position);
    return os.str();
}

std::string toString(const WorkCompletedEvent& e) {
    std::ostringstream os;
    os << "[WorkCompletedEvent] from=" << e.from
       << " kind=" << e.workKind
       << " where=" << posStr(e.where)
       << " ok=" << (e.success ? "true" : "false");
    return os.str();
}

// -------- Commands --------

std::string toString(const MoveCommand& c) {
    std::ostringstream os;
    os << "[MoveCommand] to=" << c.to
       << " dst=" << posStr(c.dst);
    return os.str();
}

std::string toString(const StartWorkCommand& c) {
    std::ostringstream os;
    os << "[StartWorkCommand] to=" << c.to
       << " kind=" << c.kind;
    return os.str();
}

std::string toString(const StopCommand& c) {
    std::ostringstream os;
    os << "[StopCommand] to=" << c.to;
    return os.str();
}

std::string toString(const TickCommand& c) {
    std::ostringstream os;
    os << "[TickCommand] now=" << c.now;
    return os.str();
}
