#pragma once
#include "common.hpp"
#include "astar.hpp"
#include <vector>
#include "visitor.hpp"

class Cell;
class Field;

const size_t MID_SPEED = 50;
const size_t HIGH_SPEED = 100;
const size_t LOW_SPEED = 10;

class Passenger : public GameObject
{
    size_t x_;
    size_t y_;

    size_t speed_ = MID_SPEED;
    size_t aim_ind_;
    std::vector<size_t> path_;
    StatusTypes status_ = FIND_REG;

    Field& field_;

    PathSituation step_status_;

public:

    Passenger(Field& field);
    Passenger(size_t speed, Field& field);

    size_t get_aim () const;
    const std::vector<size_t>& get_path () const;
    size_t get_x() const;
    size_t get_y() const;
    StatusTypes get_status() const;
    size_t get_next_step() const;
    void set_x(size_t x) {x_ = x;}
    void set_y(size_t y) {y_ = y;}
    void set_status(StatusTypes new_status) {status_ = new_status;}
    void set_aim(size_t aim);

    void plan_step();
    void make_step();
    void simple_step(std::pair<size_t, size_t> next_coord);
    void get_in_line();
    void get_around_opponent(size_t cur_ind, size_t opponents_ind);
    void get_around_queue(size_t cur_ind);
    
    void find_reg();
    size_t reg();
    void find_gate();
    void end_of_path();

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};

//-----------------------------------------------------------

/*class Busy : public Passenger
{
public:
    Busy(Field& field);

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};

//-----------------------------------------------------------

class Old : public Passenger
{
public:

    Old(Field& field);

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};*/