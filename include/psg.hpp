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

enum PathSituation
{
    FREE,
    COLLISION,
    QUEUE,
    PASSING_BY,
    OTHER_INTERESTED,
    QUEUE_OBSTACLE
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

    class PathController
    {
        Field& field_;
        Passenger& psg_;

    public:

        void make_step()
        {
            std::vector<size_t> path = psg_.get_path();
            size_t next_cell_ind = path[1];
            PathSituation step_status = check_next_step(next_cell_ind);
            std::pair<size_t, size_t> next_coord = f1dto2d(next_cell_ind, field_.get_wid());
            size_t cur_ind = path[0];

            switch(step_status)
            {
                case PASSING_BY:
                case FREE:
                {
                    step(next_coord);
                    break;
                }
                case COLLISION:
                {
                    int random = rand() % 2;
                    if (random)
                    {
                        get_around_opponent(cur_ind, next_cell_ind);
                        return;
                    }
                    //wait_for_step();  можно без функции отдельной, просто пропускать шаг
                    break;
                }
                case QUEUE:
                {
                    get_in_line();
                    break;
                }
                case OTHER_INTERESTED:
                {
                    //просто ждем эту итерацию
                    break;
                }
                case QUEUE_OBSTACLE:
                {
                    get_around_queue(cur_ind);
                }
            }
        }

        void step(std::pair<size_t, size_t> next_coord)
        {
            psg_.set_x(next_coord.first);
            psg_.set_y(next_coord.second);
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

        const std::vector<std::pair<int, int>> steps = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

        bool is_there_others_interested(size_t next_step_ind)
        {
            std::pair<int, int> cur_coord = f1dto2d(next_step_ind, field_.get_wid());

            for (auto& step : steps)
            {
                std::pair<int, int> new_coord = {cur_coord.first + step.first, cur_coord.second + step.second};
                //std::cout << "new step " << new_coord.first << ", " << new_coord.second << std::endl;

                if (new_coord.first < 0 || new_coord.second < 0)
                {
                    //std::cout << "less" << std::endl;
                    continue;
                }

                if (new_coord.first > field_.get_wid() || new_coord.second > field_.get_len())
                {
                    //std::cout << "more" << std::endl;
                    continue;
                }

                size_t cell_ind = f2dto1d(new_coord.first, new_coord.second, field_.get_wid());
                Cell* cell = field_.get_cell_by_ind(cell_ind);
                size_t opponents_aim = cell->get_psg_aim_ind();

                if (opponents_aim == next_step_ind)
                    return true;

                return false;
            }
        }

        void get_in_line()
        {
            psg_.set_status(WAITING_IN_LINE); //стоит ли делать так, чтобы он попробовал найти последнего в очереди? можно потом добавить
        }

        void get_around_opponent(size_t cur_ind, size_t opponents_ind)
        {
            std::vector<size_t> new_obstacles = update_obstacles_by_opponent(opponents_ind);
            AStarPathFinder path_finder(new_obstacles);
            std::vector<size_t> updated_path = path_finder.find_path(field_, cur_ind, psg_.get_aim());
        }

        void get_around_queue(size_t cur_ind)
        {
            std::vector<size_t> new_obstacles = update_obstacles_by_queues();
            AStarPathFinder path_finder(new_obstacles);
            std::vector<size_t> updated_path = path_finder.find_path(field_, cur_ind, psg_.get_aim());
        }

        std::vector<size_t> update_obstacles_by_opponent(size_t opponents_coord)
        {
            std::vector<size_t> field_obstacles = field_.get_obstacles();
            std::vector<size_t> obstacles;
            std::copy(field_obstacles.begin(), field_obstacles.end(), obstacles.begin());

            obstacles.push_back(opponents_coord);

            return obstacles;
        }

        std::vector<size_t> update_obstacles_by_queues()
        {
            //можно просто пройтись напрямую по всем клеткам, а не отдельными списками
            std::vector<size_t> obstacles;
            Cell* cell;

            for (size_t ind = 0 ; ind < field_.get_wid() * field_.get_len(); ind++)
            {
                cell = field_.get_cell_by_ind(ind);

                if (!cell->is_passable() || cell->is_occupied())
                    obstacles.push_back(ind);
            }

            return obstacles;
        }
    };
    
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
    void set_status(StatusTypes new_status) {status_ = new_status;}
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

//!! шагать нужно до того момента, пока мы не достигнем цели или не встанем в очередь (стоит проверять статус пассажира)
