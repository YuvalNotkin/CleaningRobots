#pragma once
#include <queue>
#include <vector>
#include "robot.hpp"       // Position, RobotType, RobotState
#include "registry.hpp"    // RobotRegistry

// Minimal job representation for the MVP (no IDs, no global map).
enum class JobStage { DETECT, VACUUM, WASH, DONE };

struct Job {
    Position target;
    JobStage stage{JobStage::DETECT};
};

class Planner {
public:
    // Push a new job into the DETECT queue.
    void enqueueDetect(Position p) { qDetect_.push(Job{p, JobStage::DETECT}); }

    // Returns true if any queue still has pending work.
    bool hasWork() const {
        return !qDetect_.empty() || !qVacuum_.empty() || !qWash_.empty();
    }

    // Execute one scheduling step (synchronous).
    // Returns true if the step made progress (some job advanced),
    // false if no suitable robot was found and the job was requeued.
    bool step(RobotRegistry& registry);

private:
    // Simple stage promotion helper.
    static JobStage nextStage(JobStage s) {
        if (s == JobStage::DETECT) return JobStage::VACUUM;
        if (s == JobStage::VACUUM) return JobStage::WASH;
        return JobStage::DONE;
    }

    // Returns (queue pointer, stage) for the next non-empty queue by priority.
    std::pair<std::queue<Job>*, JobStage> pickQueueByPriority();

    // Queues per stage (by value to keep things very simple).
    std::queue<Job> qDetect_;
    std::queue<Job> qVacuum_;
    std::queue<Job> qWash_;
};
