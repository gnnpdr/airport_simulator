#include "psg.hpp"

Passenger::Passenger(Field& field) : field_(field) {}

Passenger::Passenger(size_t speed, Field& field) : speed_(speed), field_(field)
{
    auto& enters = field_.get_enterances();
    size_t enter_ind = rand() % enters.size();
    size_t enter_coord_1d = enters[enter_ind];
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
    x_ = enter_coord_2d.first;
    y_ = enter_coord_2d.second;
}
 
 size_t Passenger::get_x() const {return x_;}
 size_t Passenger::get_y() const {return y_;}
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

//------------------------------------------------------------

void Passenger::find_reg()
{
    std::cout << "FIND_REG!!" << std::endl;
    std::cout << "cur coord = ( " << x_ << ", " << y_ << " )" << std::endl;
    size_t free_office_ind = field_.find_free_reg_office_ind();
    RegOffice* free_office = static_cast<RegOffice*>(field_.get_cell_by_ind(free_office_ind));
    std::cout << "free office coords " << free_office->get_x() << ", " << free_office->get_y() << std::endl;

    aim_ind_ = free_office_ind;
    //! это потомfree_office->take_turn();

    AStarPathFinder path_finder(field_);
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    path_ = path_finder.find_path(cur_ind, aim_ind_);
    status_ = GOING_TO_REG;
    std::cout << "path to reg office ind" << aim_ind_ << std::endl; 
    path_finder.print_path();
}

void Passenger::find_gate()
{

    RegOffice* reg_office = static_cast<RegOffice*>(field_.get_cell_by_ind(aim_ind_));
    if(status_ == GOING_TO_REG)
        reg_office->take_turn();

    aim_ind_ = reg();

    reg_office->free_queue_space();

    AStarPathFinder path_finder(field_);
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    path_ = path_finder.find_path(cur_ind, aim_ind_);
    std::cout << "path to gate ind" << aim_ind_ << std::endl; 
    path_finder.print_path();

    status_ = GOING_TO_GATE;
}

size_t Passenger::reg()
{
    printf("find gate\n");
    auto& gates = field_.get_gates();
    size_t gatenum = rand() % gates.size();
    RegOffice* the_office = static_cast<RegOffice*>(field_.get_cell_by_ind(aim_ind_));
    the_office->free_queue_space();
    return gates[gatenum]; 
}

//!! шагать нужно до того момента, пока мы не достигнем цели или не встанем в очередь (стоит проверять статус пассажира)

//-------------------------------------------------------------------------------

void Passenger::plan_step()
{
    //!нужно еще обработать случай, когда только один элемент в пути - тогда шаг делать не надо. 
    if (path_.size() == 1)
    {
        end_of_path();
        return;
    }

    size_t next_cell_ind = path_[1];
    size_t cur_ind = path_[0];
    std::cout << "size " << path_.size() << " cur ind " << cur_ind << " next_ind " << next_cell_ind << std::endl;
    step_status_ = field_.check_next_step(aim_ind_, cur_ind, next_cell_ind);

    std::cout << "step_status " << step_status_ << std::endl;
}

void Passenger::make_step()
{
    std::cout << "MAKE STEP" << std::endl;
    size_t next_cell_ind = path_[1];
    size_t cur_ind = path_[0];
    std::pair<size_t, size_t> next_coord = f1dto2d(next_cell_ind, field_.get_wid());
    
    switch(step_status_)
    {
        case PASSING_BY:
        case FREE:
        {
            printf("step\n");
            simple_step(next_coord);
            break;
        }
        case COLLISION:
        {
            printf("collision\n");
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
            printf("queue\n");
            get_in_line();
            break;
        }
        case OTHER_INTERESTED:
        {
            printf("other interested\n");
            //просто ждем эту итерацию
            break;
        }
        case QUEUE_OBSTACLE:
        {
            printf("obstacle\n");
            get_around_queue(cur_ind);
        }
        case NO_CELL_NULLPTR:
            break;
    }
}

void Passenger::simple_step(std::pair<size_t, size_t> next_coord)
{
    size_t old_ind = f2dto1d(next_coord.first, next_coord.second, field_.get_wid());
    Cell* old_cell = field_.get_cell_by_ind(old_ind);
    old_cell->set_cell_free();
    x_ = next_coord.first;
    y_ = next_coord.second;
    path_.erase(path_.begin());
    size_t ind = f2dto1d(next_coord.first, next_coord.second, field_.get_wid());
    Cell* cell = field_.get_cell_by_ind(ind);
    cell->set_psg(this);
}
    
void Passenger::get_in_line()
{
    set_status(WAITING); //стоит ли делать так, чтобы он попробовал найти последнего в очереди? можно потом добавить
}

void Passenger::get_around_opponent(size_t cur_ind, size_t opponents_ind)
{
    std::vector<size_t> new_obstacles = field_.update_obstacles_by_opponent(opponents_ind);
    AStarPathFinder path_finder(field_, new_obstacles);
    std::vector<size_t> updated_path = path_finder.find_path(cur_ind, aim_ind_);
}

void Passenger::get_around_queue(size_t cur_ind)
{
    std::vector<size_t> new_obstacles = field_.update_obstacles_by_queues();
    AStarPathFinder path_finder(field_, new_obstacles);
    std::vector<size_t> updated_path = path_finder.find_path(cur_ind, aim_ind_);
}

void Passenger::end_of_path()
{
    if (status_ == GOING_TO_REG)
        status_ = REG; 
    if (status_ == GOING_TO_GATE)
    {
        size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
        Cell* cell = field_.get_cell_by_ind(cur_ind);
        cell->set_cell_free();
    }
}

//-----------------------------------------------------------

/*Busy::Busy(Field& field) : Passenger(HIGH_SPEED, field) {}

//-----------------------------------------------------------

Old::Old(Field& field) : Passenger(LOW_SPEED, field) {}*/