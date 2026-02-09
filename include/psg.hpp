#pragma once
#include "common.hpp"
#include <vector>
#include "visitor.hpp"

const size_t MID_SPEED = 50;
const size_t HIGH_SPEED = 100;
const size_t LOW_SPEED = 10;

class Passenger : public GameObject
{
    size_t ind_;

    size_t speed_ = MID_SPEED;
    size_t aim_ind_;
    std::vector<size_t> path_;
    StatusTypes status_ = FIND_REG;
    PathSituation step_status_;

public:

    Passenger() = default;
    Passenger(size_t speed);

    size_t get_aim () const;
    const std::vector<size_t>& get_path () const;
    size_t get_ind() const;
    StatusTypes get_status() const;
    size_t get_next_step() const;
    void set_ind(size_t ind) {ind_ = ind;}
    void set_status(StatusTypes new_status) {status_ = new_status;}
    void set_aim(size_t aim);
    void set_path(std::vector<size_t> path);
    void set_step_status(PathSituation step_status);

    PathSituation get_step_status();

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