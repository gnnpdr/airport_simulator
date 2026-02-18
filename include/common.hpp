#pragma once
#include <iostream>
#include <cstddef>
#include <vector>

inline size_t f2dto1d (size_t x, size_t y, size_t wid)
{
    return y * wid + x;
}

inline std::pair<size_t, size_t> f1dto2d (size_t ind, size_t wid)
{
    size_t x = ind % wid;
    size_t y = ind / wid;

    //std::cout << "[DEBUG] f1dto2d: ind=" << ind << ", wid=" << wid 
    //          << " -> x=" << x << ", y=" << y << std::endl;

    return {x, y};
}

enum StatusTypes
{
    NULLPTR,            //хммм
    FIND_REG,
    GOING_TO_REG,
    WAITING,
    REG,
    GOING_TO_GATE
};

enum PathSituation
{
    NO_CELL_NULLPTR,        //хммм
    RESET,
    FREE,
    COLLISION_WAIT,
    COLLISION_GO,
    QUEUE,
    PASSING_BY,
    QUEUE_OBSTACLE,
    NO_MOVING,
    REALLY_REG,
    START_REG
};

const std::vector<std::pair<int, int>> steps = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

const size_t ERROR_VAL = 888;