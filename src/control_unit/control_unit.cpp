#include <iostream>
#include <cstdlib>
#include <limits>
#include <set>
#include <utility>
#include <vector>
#include <variant>
#include <type_traits>
#include "control_unit/control_unit.hpp"

// ---- helper functions inside anonymous namespace ----
namespace {
using CellKey = std::pair<int, int>;
CellKey cellKey(Position p) {
    return {p.x, p.y};
}
bool samePosition(Position a, Position b) {
    return a.x == b.x && a.y == b.y;
}
// can be optimized later if needed
int distance(Position a, Position b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}
}

// ---- command helpers - create and send commands via the bus ----

void ControlUnit::sendMoveCmd(RobotId id, Position dst) {
    MoveCommand cmd;
    cmd.to = id;
    cmd.position = dst;
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

// ---- event processing ----

void ControlUnit::drainEvents() {
    Bus::EventVariant event;
    while (bus_.poll(event)) {
        handleEvent(event);
    }
}

// ---- event handling helpers ----

// dispatch event to the appropriate handler based on its type
void ControlUnit::handleEvent(const Bus::EventVariant& event) {
    // lambda visitor to process each event type
    std::visit([this](const auto& ev) {
        using EventType = std::decay_t<decltype(ev)>;
        if constexpr (std::is_same_v<EventType, StatusEvent>) {
            handleStatusEvent(ev);
        } else if constexpr (std::is_same_v<EventType, WorkCompletedEvent>) {
            handleWorkCompletedEvent(ev);
        } else if constexpr (std::is_same_v<EventType, DetectionEvent>) {
            // Detection events not used currently, the detection simulation is implicit in the CU logic for now
            (void)ev;
        }
    }, event);
}

// handle status event - currently only process ARRIVED state
void ControlUnit::handleStatusEvent(const StatusEvent& event) {
    // only care about ARRIVED state for now
    if (event.state != RobotState::ARRIVED) {
        return;
    }

    // find pending task for this robot
    auto it = pendingTasks_.find(event.from);
    if (it == pendingTasks_.end()) {
        return;
    }

    // start the work
    const PendingTask& task = it->second;
    std::cout << "[CU] Robot " << event.from << " arrived at ("
              << event.position.x << "," << event.position.y
              << ") -> start " << task.kind << "\n";
    sendStartRobotWorkCmd(event.from, task.kind);
}

// handle work completed event
void ControlUnit::handleWorkCompletedEvent(const WorkCompletedEvent& event) {
    std::cout << "[CU] Robot " << event.from << " completed "
              << event.workKind << " at ("
              << event.position.x << "," << event.position.y << ")"
              << (event.success ? "" : " with failure")
              << "\n";

    // remove from pending tasks
    auto taskIt = pendingTasks_.find(event.from);
    if (taskIt != pendingTasks_.end()) {
        pendingTasks_.erase(taskIt);
    }

    // check success
    if (!event.success) {
        return;
    }

    // post-process based on work kind
    if (event.workKind == "VACUUM") {
        if (map_.markVacuumed(event.position)) {
            if (queuedForWasher_.insert(cellKey(event.position)).second) {
                washerQueue_.push(event.position);
            }
        }
    } else if (event.workKind == "WASH") {
        map_.markWashed(event.position);
    }
}

// ---- utility functions, not in use yet ----

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

// create environment map (size and dirt spots) and configure planner
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

    // check planner configured
    if (!planner_.isConfigured()) {
        std::cerr << "[CU] planner not configured. Did you call seedFrom()?\n";
        return;
    }

    // Resetting queues and bookkeeping
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

    // Advancing each Detector one step according to the assigned path (Lambda):
    auto processDetectors = [&]() -> bool {
        bool madeProgress = false;

        // process each detector state
        for (std::size_t i = 0; i < detectors.size(); ++i) {
            DetectorState& state = detectors[i];
            // if not started yet
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
            // get next cell in path
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
        // Each iteration tries to advance detectors and assign tasks to vacuums and washers
        bool detectorsProgress = processDetectors();
        bool vacuumProgress = processVacuumQueue();
        bool washerProgress = processWasherQueue();

        // check if all detectors are finished
        bool allDetectorsFinished = true;
        for (const auto& state : detectors) {
            if (!state.finished) {
                allDetectorsFinished = false;
                break;
            }
        }

        // termination condition: all detectors finished and no pending tasks
        if (allDetectorsFinished && vacuumQueue_.empty() && washerQueue_.empty()) {
            break;
        }

        // safety check: if no progress made in this iteration, break to avoid infinite loop
        if (!detectorsProgress && !vacuumProgress && !washerProgress) {
            std::cerr << "[CU] run loop made no progress; aborting.\n";
            break;
        }
    }
}

bool ControlUnit::processVacuumQueue() {
    bool processed = false;
    // Try to assign tasks to idle vacuum robots
    while (!vacuumQueue_.empty()) {
        Position target = vacuumQueue_.front();
        // Check if Dirty - maybe already vacuumed
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
        // Remove from queue
        vacuumQueue_.pop();
        queuedForVacuum_.erase(cellKey(target));
        // Assign task and send command
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
        // Check if Dirty - maybe already washed
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
        // Remove from queue
        washerQueue_.pop();
        queuedForWasher_.erase(cellKey(target));
        // Assign task and send command
        pendingTasks_[robot->id()] = PendingTask{"WASH", target};
        sendMoveCmd(robot->id(), target);
        drainEvents();

        processed = true;
    }

    return processed;
}

// enqueue a CELL for vacuuming to the vacuumQueue_ 
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

// find the nearest idle robot of the given type to the target position
std::shared_ptr<RobotBase> ControlUnit::findNearestIdleRobot(RobotType type, Position target) const {
    auto robots = reg_.getByType(type);
    std::shared_ptr<RobotBase> best;
    int bestDistance = std::numeric_limits<int>::max();
    // search among idle robots without pending tasks - use distance method to find the nearest one
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
