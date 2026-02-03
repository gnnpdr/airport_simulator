#pragma once
#include "astar.hpp"

const size_t MID_SPEED = 50;
const size_t HIGH_SPEED = 100;
const size_t LOW_SPEED = 10;

enum StatusTypes
{
    WAITING, 
    CHOOSING_DESK,
    GOING_TO_DESK,
    GOING_TO_GATE,
    REG
};

class Passenger
{
    size_t x_;
    size_t y_;

    size_t speed_ = MID_SPEED;
    size_t aim_ind_;
    std::vector<size_t> path_;
    StatusTypes status_ = CHOOSING_DESK;

    Field& field_;
    
public:

    Passenger(Field& field) : field_(field) 
    {
        auto& enters = field_.get_enterances();
        size_t enter_ind = rand() % (enters.size() + 1);
        size_t enter_coord_1d = enters[enter_ind];
        std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
        x_ = enter_coord_2d.first;
        y_ = enter_coord_2d.second;
    }
    Passenger(size_t speed, Field& field) : speed_(speed), field_(field)
    {
        auto& enters = field_.get_enterances();
        size_t enter_ind = rand() % (enters.size() + 1);
        size_t enter_coord_1d = enters[enter_ind];
        std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
        x_ = enter_coord_2d.first;
        y_ = enter_coord_2d.second;
    }
 
    const size_t get_aim () const {return aim_ind_;}
    const std::vector<size_t>& get_path () const {return path_;}

    void set_aim(size_t aim)
    {
        aim_ind_ = aim;
    }

    void start_algo()
    {
        size_t free_office_ind = field_.find_free_reg_office_ind();
        std::cout << "free office ind " << free_office_ind << std::endl;
        RegOffice* free_office = static_cast<RegOffice*>(field_.get_cell_by_ind(free_office_ind));
        std::cout << "free office coords " << free_office->get_x() << ", " << free_office->get_y() << std::endl;

        aim_ind_ = free_office_ind;
        free_office->take_turn();

        AStarPathFinder path_finder;
        size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
        path_ = path_finder.find_path(field_, cur_ind, aim_ind_);
        std::cout << "path to reg office ind" << aim_ind_ << std::endl; 
        path_finder.print_path(field_);
        move();
    }

    void move()
    {
        size_t wid = field_.get_wid();
        while (!path_.empty())
        {
            size_t new_ind = path_.front();
            path_.erase(path_.begin());
            std::pair<size_t, size_t> new_coord = f1dto2d(new_ind, wid);
            x_ = new_coord.first;
            y_ = new_coord.second;
        }
    }

    size_t reg_proc()
    {
        auto& gates = field_.get_gates();
        size_t gatenum = rand() % (gates.size() + 1);
        RegOffice* the_office = static_cast<RegOffice*>(field_.get_cell_by_ind(aim_ind_));
        the_office->free_queue_space();
        return gates[gatenum]; 
    }

    void end_algo()
    {
        aim_ind_ = reg_proc();
        AStarPathFinder path_finder;
        size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
        path_ = path_finder.find_path(field_, cur_ind, aim_ind_);
        std::cout << "path to gate ind" << aim_ind_ << std::endl; 
        path_finder.print_path(field_);
        move();
    }

    const size_t get_x() const {return x_;}
    const size_t get_y() const {return y_;}
};

class Busy : public Passenger
{
public:
    Busy(Field& field) : Passenger(HIGH_SPEED, field) {}
};

class Old : public Passenger
{
public:
    Old(Field& field) : Passenger(LOW_SPEED, field) {}
};

