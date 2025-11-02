#include "include/planner.hpp"
#include <iostream>  // for simple logs

// Pick the first non-empty queue in fixed priority order: DETECT -> VACUUM -> WASH.
std::pair<std::queue<Job>*, JobStage> Planner::pickQueueByPriority() {
    if (!qDetect_.empty()) return { &qDetect_, JobStage::DETECT };
    if (!qVacuum_.empty()) return { &qVacuum_, JobStage::VACUUM };
    if (!qWash_.empty())   return { &qWash_,   JobStage::WASH   };
    return { nullptr, JobStage::DONE };
}

bool Planner::step(RobotRegistry& registry) {
    auto [q, stage] = pickQueueByPriority();
    if (!q) return false; // nothing to do

    // Take one job from the chosen queue
    Job job = q->front();
    q->pop();

    // Decide the required robot type for this stage
    RobotType need = RobotType::DETECTOR;
    const char* workKind = "DETECT";
    if (stage == JobStage::VACUUM) { need = RobotType::VACUUM; workKind = "VACUUM"; }
    else if (stage == JobStage::WASH) { need = RobotType::WASHER; workKind = "WASH"; }

    // Try local search first (small radius), then a wide fallback.
    auto pick = registry.findNearest(need, job.target, /*maxRadiusCells=*/2);
    if (!pick || pick->state() != RobotState::IDLE) {
        pick = registry.findNearest(need, job.target, /*fallback wide*/ 1000000);
        if (!pick || pick->state() != RobotState::IDLE) {
            // No suitable robot right now -> requeue the job and report no progress.
            // This keeps the code simple and avoids busy-wait; next loop will try again.
            if (stage == JobStage::DETECT) qDetect_.push(job);
            else if (stage == JobStage::VACUUM) qVacuum_.push(job);
            else qWash_.push(job);
            return false;
        }
    }

    // Synchronous "do the work" sequence (no sleeps, no threads, just simulation):
    pick->moveTo(job.target);
    registry.updatePosition(pick->id(), job.target);

    pick->startWork(workKind);
    // In this MVP we consider work done immediately to keep the flow simple.
    pick->stop();

    // Promote job to the next stage or finish
    JobStage next = nextStage(stage);
    if (next == JobStage::DONE) {
        std::cout << "[Planner] Job at (" << job.target.x << "," << job.target.y << ") DONE\n";
    } else {
        job.stage = next;
        if (next == JobStage::VACUUM) qVacuum_.push(job);
        else                          qWash_.push(job);
    }

    return true; // made progress
}
