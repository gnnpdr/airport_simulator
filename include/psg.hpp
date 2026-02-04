#pragma once
#include "astar.hpp"

class Cell;
class Field;

const size_t MID_SPEED = 50;
const size_t HIGH_SPEED = 100;
const size_t LOW_SPEED = 10;

enum StatusTypes
{
    WAITING_IN_LINE, 
    CHOOSING_DESK,
    GOING_TO_DESK,
    GOING_TO_GATE,
    //REG
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

    Passenger(Field& field);
    Passenger(size_t speed, Field& field);
 
    const size_t get_aim () const;
    const std::vector<size_t>& get_path () const;

    void set_aim(size_t aim);

    void start_algo();

    void move();
    size_t reg_proc();

    void end_algo();

    const size_t get_x() const;
    const size_t get_y() const;

    const StatusTypes get_status() const;
    const size_t get_next_step() const;

    void set_x(size_t x) {x_ = x;}
    void set_y(size_t y) {y_ = y;}
};

//-----------------------------------------------------------

class Busy : public Passenger
{
public:
    Busy(Field& field);
};

//-----------------------------------------------------------

class Old : public Passenger
{
public:
    Old(Field& field);
};


//--------------------------------------------

enum PathSituation
{
    FREE,
    COLLISION,
    QUEUE,
    PASSING_BY,
    OTHER_INTERESTED,
    QUEUE_OBSTACLE
};

class PathController
{
    Field& field_;
    Passenger& psg_;

public:

    void make_step()
    {
        std::vector<size_t> path = psg_.get_path();
        PathSituation step_status = check_next_step(path[1]);
        std::pair<size_t, size_t> next_coord = f1dto2d(path[1], field_.get_wid());

        switch(step_status)
        {
            case PASSING_BY:
            case FREE:
            {
                psg_.set_x(next_coord.first);
                psg_.set_y(next_coord.second);
                break;
            }
            case COLLISION:
            {
                int random = rand() % 2;
                if (random)
                {
                    get_around();
                    return;
                }
                wait_for_step();
                break;
            }
            case QUEUE:
            {
                get_in_line();
                break;
            }
            case OTHER_INTERESTED:
            {
                wait();
                break;
            }
            case QUEUE_OBSTACLE:
            {
                std::vector<size_t> new_obstacles = make_new_obstacle_list();
                AStarPathFinder path_finder()
                std::vector<size_t> updated_path = astart
            }
        }
    }

    PathSituation check_next_step(size_t next_step_ind)
    {
        Cell* cell = field_.get_cell_by_ind(next_step_ind);

        if (!cell->is_occupied())
        {
            if (!is_there_others_interested(next_step_ind))
                return FREE;
            return OTHER_INTERESTED;
        }

        StatusTypes opponent_status = cell->get_psg_status();
        
        if (opponent_status == WAITING_IN_LINE)
        {
            size_t opponents_aim = cell->get_psg_aim_ind();
            if (opponents_aim == psg_.get_aim())
                return QUEUE;
            return QUEUE_OBSTACLE;      //сразу учитывается и то, что я не встаю в очередь, потому что иду к другой будке, и потому что иду к выходу
        }

        size_t opponent_next_step_ind = cell->get_next_psg_step();
        size_t my_ind = f2dto1d(psg_.get_x(), psg_.get_y(), field_.get_wid());
        if (my_ind == opponent_next_step_ind)
            return COLLISION;

        return PASSING_BY;
    }

    bool is_there_others_interested(size_t ind);
    void get_in_line();
    void get_around();
    void wait_for_step();
};