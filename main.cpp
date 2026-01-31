#include <iostream>
#include "place.hpp"

int main()
{
    std::vector<std::pair<size_t, size_t>> obstacles = {{0, 0}, {5, 5}};
    std::vector<std::pair<size_t, size_t>> reg_offices = {{2, 3}, {4, 3}};
    std::vector<std::pair<size_t, size_t>> gates = {{5, 0}, {5, 3}};
    std::vector<std::pair<size_t, size_t>> enters = {{1, 0}, {3, 0}};

    Field field(6, 6, obstacles, reg_offices, enters, gates);

    field.draw();

    return 0;
}