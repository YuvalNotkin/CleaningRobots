#pragma once
#include <atomic>
#include "common/types.hpp"

struct IdGenerator {
    static RobotId next() {
        static std::atomic<RobotId> counter{1};
        return counter++;
    }
};
