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

    return {x, y};
}

enum StatusTypes
{
    NULLPTR,
    WAITING_IN_LINE, 
    CHOOSING_DESK,
    GOING_TO_DESK,
    GOING_TO_GATE,
    //REG
};

enum PathSituation
{
    NO_CELL_NULLPTR,
    FREE,
    COLLISION,
    QUEUE,
    PASSING_BY,
    OTHER_INTERESTED,
    QUEUE_OBSTACLE
};

const std::vector<std::pair<int, int>> steps = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

const size_t ERROR_VAL = 888;