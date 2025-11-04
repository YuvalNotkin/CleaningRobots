#include <iostream>
#include <cstdlib>
#include <limits>
#include <set>
#include <utility>
#include <vector>
#include <variant>
#include <type_traits>
#include "control_unit/control_unit.hpp"

namespace {
using CellKey = std::pair<int, int>;
CellKey cellKey(Position p) {
    return {p.x, p.y};
}
bool samePosition(Position a, Position b) {
    return a.x == b.x && a.y == b.y;
}
int distance(Position a, Position b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
}

// ---- command helpers ----
void ControlUnit::sendMoveCmd(RobotId id, Position dst) {
    MoveCommand cmd;
    cmd.to = id;
    cmd.dst = dst;
    bus_.broadcast(std::move(cmd));
}

void ControlUnit::sendStartRobotWorkCmd(RobotId id, const std::string& kind) {
    StartWorkCommand cmd;
    cmd.to = id;
    cmd.kind = kind;
    bus_.broadcast(std::move(cmd));
}

void ControlUnit::sendStopRobotCmd(RobotId id) {
    StopCommand cmd;
    cmd.to = id;
    bus_.broadcast(std::move(cmd));
}

void ControlUnit::drainEvents() {
    Bus::EventVariant event;
    while (bus_.poll(event)) {
        handleEvent(event);
    }
}

void ControlUnit::handleEvent(const Bus::EventVariant& event) {
    std::visit([this](const auto& ev) {
        using EventType = std::decay_t<decltype(ev)>;
        if constexpr (std::is_same_v<EventType, StatusEvent>) {
            handleStatusEvent(ev);
        } else if constexpr (std::is_same_v<EventType, WorkCompletedEvent>) {
            handleWorkCompletedEvent(ev);
        } else if constexpr (std::is_same_v<EventType, DetectionEvent>) {
            // Detection events can be handled later if needed.
            (void)ev;
        }
    }, event);
}

void ControlUnit::handleStatusEvent(const StatusEvent& event) {
    if (event.state != RobotState::ARRIVED) {
        return;
    }

    auto it = pendingTasks_.find(event.from);
    if (it == pendingTasks_.end()) {
        return;
    }

    const PendingTask& task = it->second;
    std::cout << "[CU] Robot " << event.from << " arrived at ("
              << event.position.x << "," << event.position.y
              << ") -> start " << task.kind << "\n";
    sendStartRobotWorkCmd(event.from, task.kind);
}

void ControlUnit::handleWorkCompletedEvent(const WorkCompletedEvent& event) {
    std::cout << "[CU] Robot " << event.from << " completed "
              << event.workKind << " at ("
              << event.where.x << "," << event.where.y << ")"
              << (event.success ? "" : " with failure")
              << "\n";

    auto taskIt = pendingTasks_.find(event.from);
    if (taskIt != pendingTasks_.end()) {
        pendingTasks_.erase(taskIt);
    }

    if (!event.success) {
        return;
    }

    if (event.workKind == "VACUUM") {
        if (map_.markVacuumed(event.where)) {
            if (queuedForWasher_.insert(cellKey(event.where)).second) {
                washerQueue_.push(event.where);
            }
        }
    } else if (event.workKind == "WASH") {
        map_.markWashed(event.where);
    }
}

void ControlUnit::printRobots() const {
    auto printVec = [](const std::vector<std::shared_ptr<RobotBase>>& vec) {
        for (const auto& r : vec) {
            std::cout << "[CU] Robot ID=" << r->id()
                      << " Type="  << static_cast<int>(r->type())
                      << " State=" << static_cast<int>(r->state()) << "\n";
        }
    };
    printVec(reg_.getByType(RobotType::DETECTOR));
    printVec(reg_.getByType(RobotType::VACUUM));
    printVec(reg_.getByType(RobotType::WASHER));
}

///////////////////////////////////////// MAIN SIMULATION ////////////////////////////////////////////////////////

// Creating dirt
void ControlUnit::seedFrom(const BootstrapFeed& feed) {
    if (!map_.initializeGrid(feed.gridWidth, feed.gridHeight, feed.dirtSpots)) {
        std::cerr << "[CU] failed to initialize grid; aborting scenario.\n";
        return;
    }
    planner_.configureGrid(feed.gridWidth, feed.gridHeight);
}


// Running
void ControlUnit::run() {

    //////////////////// Initializing part:

    if (!planner_.isConfigured()) {
        std::cerr << "[CU] planner not configured. Did you call seedFrom()?\n";
        return;
    }

    vacuumQueue_ = std::queue<Position>{};
    washerQueue_ = std::queue<Position>{};
    queuedForVacuum_.clear();
    queuedForWasher_.clear();
    pendingTasks_.clear();
    drainEvents();

    // Assigning plan to each Detector
    auto detectorsVec = reg_.getByType(RobotType::DETECTOR);
    if (detectorsVec.empty()) {
        std::cerr << "[CU] no detector robots available\n";
        return;
    }
    auto plans = planner_.buildScanPlans(detectorsVec.size());
    if (plans.size() != detectorsVec.size()) {
        std::cerr << "[CU] unable to build scan plans\n";
        return;
    }

    // Bookkeeping block that packages each detector state in one "detectors" vector:
    struct DetectorState {
        std::shared_ptr<RobotBase> robot;
        std::vector<Position>      path;
        std::size_t                nextIndex{0};
        bool                       started{false};
        bool                       finished{false};
    };
    std::vector<DetectorState> detectors;
    detectors.reserve(detectorsVec.size());
    for (std::size_t idx = 0; idx < detectorsVec.size(); ++idx) {
        detectors.push_back(DetectorState{
            detectorsVec[idx],
            plans[idx],
            0,
            false,
            false
        });
    }

    const Position start = {0, 0};

    //////////////////// Advancing each Detector one step according to the assigned path (Lambda):

    auto processDetectors = [&]() -> bool {
        bool madeProgress = false;

        for (std::size_t i = 0; i < detectors.size(); ++i) {
            DetectorState& state = detectors[i];
            if (!state.started) {
                std::cout << "[Detector#" << state.robot->name() << "] start scan\n";
                state.started = true;
                madeProgress = true;
            }

            // if finished already
            if (state.finished) {
                continue;
            }

            // if Path ended
            if (state.nextIndex >= state.path.size()) {
                if (!samePosition(state.robot->position(), start)) {
                    sendMoveCmd(state.robot->id(), start);
                    drainEvents();
                    madeProgress = true;
                }
                state.finished = true;
                continue;
            }

            // The advancing
            Position cell = state.path[state.nextIndex++];
            madeProgress = true;

            // Move to position
            if (!samePosition(state.robot->position(), cell)) {
                sendMoveCmd(state.robot->id(), cell);
                drainEvents();
            }

            // Call vacuum robot if needed
            if (map_.hasDirt(cell)) {
                if (enqueueVacuumTask(cell)) {
                    std::cout << "[Detector#" << state.robot->name()
                              << "] detected dirt at (" << cell.x << "," << cell.y << ")\n";
                }
            }
        }

        return madeProgress;
    };

    //////////////////// The Main Loop:

    while (true) {
        bool detectorsProgress = processDetectors();
        bool vacuumProgress = processVacuumQueue();
        bool washerProgress = processWasherQueue();

        bool allDetectorsFinished = true;
        for (const auto& state : detectors) {
            if (!state.finished) {
                allDetectorsFinished = false;
                break;
            }
        }

        if (allDetectorsFinished && vacuumQueue_.empty() && washerQueue_.empty()) {
            break;
        }

        if (!detectorsProgress && !vacuumProgress && !washerProgress) {
            std::cerr << "[CU] run loop made no progress; aborting.\n";
            break;
        }
    }
}

bool ControlUnit::processVacuumQueue() {
    bool processed = false;

    while (!vacuumQueue_.empty()) {
        Position target = vacuumQueue_.front();
        // Check if Dirty
        if (!map_.hasDirt(target)) {
            queuedForVacuum_.erase(cellKey(target));
            vacuumQueue_.pop();
            continue;
        }

        // Find the right Robot
        auto robot = findNearestIdleRobot(RobotType::VACUUM, target);
        if (!robot) {
            break;
        }

        vacuumQueue_.pop();
        queuedForVacuum_.erase(cellKey(target));

        pendingTasks_[robot->id()] = PendingTask{"VACUUM", target};
        sendMoveCmd(robot->id(), target);
        drainEvents();

        processed = true;
    }

    return processed;
}

bool ControlUnit::processWasherQueue() {
    bool processed = false;

    while (!washerQueue_.empty()) {
        Position target = washerQueue_.front();
        // Check if Dirty
        if (!map_.needsWash(target)) {
            queuedForWasher_.erase(cellKey(target));
            washerQueue_.pop();
            continue;
        }

        // Find the right Robot
        auto robot = findNearestIdleRobot(RobotType::WASHER, target);
        if (!robot) {
            break;
        }

        washerQueue_.pop();
        queuedForWasher_.erase(cellKey(target));

        pendingTasks_[robot->id()] = PendingTask{"WASH", target};

        sendMoveCmd(robot->id(), target);
        drainEvents();

        processed = true;
    }

    return processed;
}

// Add to vacuum list 
bool ControlUnit::enqueueVacuumTask(Position pos) {
    if (!map_.hasDirt(pos)) {
        return false;
    }
    if (queuedForVacuum_.insert(cellKey(pos)).second) {
        vacuumQueue_.push(pos);
        return true;
    }
    return false;
}

std::shared_ptr<RobotBase> ControlUnit::findNearestIdleRobot(RobotType type, Position target) const {
    auto robots = reg_.getByType(type);
    std::shared_ptr<RobotBase> best;
    int bestDistance = std::numeric_limits<int>::max();

    for (const auto& robot : robots) {
        if (robot->state() != RobotState::IDLE) {
            continue;
        }
        if (pendingTasks_.count(robot->id()) > 0) {
            continue;
        }
        const int dist = distance(robot->position(), target);
        if (dist < bestDistance) {
            bestDistance = dist;
            best = robot;
        }
    }

    return best;
}
