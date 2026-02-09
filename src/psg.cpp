#include "psg.hpp"

Passenger::Passenger(size_t speed) : speed_(speed) {}
 
size_t Passenger::get_ind() const {return ind_;}
StatusTypes Passenger::get_status() const {return status_;}
size_t Passenger::get_aim () const {return aim_ind_;}
const std::vector<size_t>& Passenger::get_path () const {return path_;}

size_t Passenger::get_next_step() const 
{
    return path_[0];
}

void Passenger::set_aim(size_t aim)
{
    aim_ind_ = aim;
}

void Passenger::set_path(std::vector<size_t> path)
{
    path_ = path;
}

void Passenger::set_step_status(PathSituation step_status)
{
    step_status_ = step_status;
}

PathSituation Passenger::get_step_status()
{
    return step_status_;
}

/*void Passenger::get_around_opponent(size_t cur_ind, size_t opponents_ind)
{
    std::vector<size_t> new_obstacles = field_.update_obstacles_by_opponent(opponents_ind);
    AStarPathFinder path_finder(field_, new_obstacles);
    std::vector<size_t> updated_path = path_finder.find_path(cur_ind, aim_ind_);
}*/

//-----------------------------------------------------------

/*Busy::Busy(Field& field) : Passenger(HIGH_SPEED, field) {}

//-----------------------------------------------------------

Old::Old(Field& field) : Passenger(LOW_SPEED, field) {}*/