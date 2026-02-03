#include <iostream>
#include "place.hpp"
#include "psg.hpp"

int main()
{
    std::vector<std::pair<size_t, size_t>> obstacles = {{0, 0}, {5, 5}};
    std::vector<std::pair<size_t, size_t>> reg_offices = {{2, 3}, {4, 3}};
    std::vector<std::pair<size_t, size_t>> gates = {{5, 0}, {5, 3}};
    std::vector<std::pair<size_t, size_t>> enters = {{1, 0}, {3, 0}};

    Field field(6, 6, obstacles, reg_offices, enters, gates);

    field.draw();

    Passenger p1(field);
    std::cout << "p coords " << p1.get_x() << ", " << p1.get_y() << std::endl;
    p1.start_algo();
    p1.end_algo();

    return 0;
}