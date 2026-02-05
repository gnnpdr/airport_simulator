#include "common.hpp"
#include "psg.hpp"

Passenger::Passenger(Field& field) : field_(field) 
{
    auto& enters = field_.get_enterances();
    size_t enter_ind = rand() % (enters.size() + 1);
    size_t enter_coord_1d = enters[enter_ind];
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
    x_ = enter_coord_2d.first;
    y_ = enter_coord_2d.second;
}

Passenger::Passenger(size_t speed, Field& field) : speed_(speed), field_(field)
{
    auto& enters = field_.get_enterances();
    size_t enter_ind = rand() % (enters.size() + 1);
    size_t enter_coord_1d = enters[enter_ind];
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
    x_ = enter_coord_2d.first;
    y_ = enter_coord_2d.second;
}
 
const size_t Passenger::get_aim () const {return aim_ind_;}
const std::vector<size_t>& Passenger::get_path () const {return path_;}

void Passenger::set_aim(size_t aim)
{
    aim_ind_ = aim;
}

void Passenger::start_algo()
{
    size_t free_office_ind = field_.find_free_reg_office_ind();
    std::cout << "free office ind " << free_office_ind << std::endl;
    RegOffice* free_office = static_cast<RegOffice*>(field_.get_cell_by_ind(free_office_ind));
    std::cout << "free office coords " << free_office->get_x() << ", " << free_office->get_y() << std::endl;

    aim_ind_ = free_office_ind;
    free_office->take_turn();

    AStarPathFinder path_finder(field_);
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    path_ = path_finder.find_path(field_, cur_ind, aim_ind_);
    std::cout << "path to reg office ind" << aim_ind_ << std::endl; 
    path_finder.print_path(field_);
    move();
}

//void Passenger::move()
//{
//    size_t wid = field_.get_wid();
//    while (!path_.empty())
//    {
//        size_t new_ind = path_.front();
//        path_.erase(path_.begin());
//        std::pair<size_t, size_t> new_coord = f1dto2d(new_ind, wid);
//        x_ = new_coord.first;
//        y_ = new_coord.second;
//    }
//}

void Passenger::move()
{
    PathController path_controller(field_, this);
}

size_t Passenger::reg_proc()
{
    auto& gates = field_.get_gates();
    size_t gatenum = rand() % (gates.size() + 1);
    RegOffice* the_office = static_cast<RegOffice*>(field_.get_cell_by_ind(aim_ind_));
    the_office->free_queue_space();
    return gates[gatenum]; 
}

void Passenger::end_algo()
{
    aim_ind_ = reg_proc();
    AStarPathFinder path_finder(field_);
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    path_ = path_finder.find_path(field_, cur_ind, aim_ind_);
    std::cout << "path to gate ind" << aim_ind_ << std::endl; 
    path_finder.print_path(field_);
    move();
}

const size_t Passenger::get_x() const {return x_;}
const size_t Passenger::get_y() const {return y_;}
const StatusTypes Passenger::get_status() const {return status_;}
const size_t Passenger::get_next_step() const 
{
    return path_[0];
}

//-----------------------------------------------------------

Busy::Busy(Field& field) : Passenger(HIGH_SPEED, field) {}

//-----------------------------------------------------------

Old::Old(Field& field) : Passenger(LOW_SPEED, field) {}

//---------------------------------