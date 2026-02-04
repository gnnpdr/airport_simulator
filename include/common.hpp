#pragma once
#include <iostream>

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