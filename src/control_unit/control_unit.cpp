#include <iostream>
#include <cstdlib>
#include <limits>
#include <set>
#include <utility>
#include <vector>
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

// ---- simple helpers ----
void ControlUnit::moveRobot(RobotId id, Position dst) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->moveTo(dst);

    // for (size_t i = 0; i < count; i++)
    // {
    //     /* code */
    // }
    
}
void ControlUnit::startRobotWork(RobotId id, const std::string& kind) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->startWork(kind);
}
void ControlUnit::stopRobot(RobotId id) {
    auto r = reg_.getById(id);
    if (!r) { std::cerr << "[CU] robot " << id << " not found\n"; return; }
    r->stop();
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
                    moveRobot(state.robot->id(), start);
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
                moveRobot(state.robot->id(), cell);
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

        // Move to target
        if (!samePosition(robot->position(), target)) {
            moveRobot(robot->id(), target);
        }

        // Vacuum
        std::cout << "[Vacuum#" << robot->name() << "] vacuuming ("
                  << target.x << "," << target.y << ")\n";
        startRobotWork(robot->id(), "VACUUM");
        stopRobot(robot->id());

        // Add to wash list 
        if (map_.markVacuumed(target)) {
            if (queuedForWasher_.insert(cellKey(target)).second) {
                washerQueue_.push(target);
            }
        }

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

        // Move to target
        if (!samePosition(robot->position(), target)) {
            moveRobot(robot->id(), target);
        }

        // Wash
        std::cout << "[Washer#" << robot->name() << "] washing ("
                  << target.x << "," << target.y << ")\n";
        startRobotWork(robot->id(), "WASH");
        stopRobot(robot->id());

        map_.markWashed(target);
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
        const int dist = distance(robot->position(), target);
        if (dist < bestDistance) {
            bestDistance = dist;
            best = robot;
        }
    }

    return best;
}
