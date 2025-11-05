#pragma once
#include <string>

using RobotId = int;
using RobotName = std::string;

struct Position {
    int x{0}, y{0};
};

enum class RobotState {
    IDLE,
    MOVING,
    ARRIVED,
    WORKING,
    ERROR // Not used currently
};

enum class RobotType  {
    DETECTOR, VACUUM, WASHER
};
