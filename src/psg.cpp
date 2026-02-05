#include "psg.hpp"

Passenger::Passenger(Field& field) : field_(field) 
{
    auto& enters = field_.get_enterances();
    size_t enter_ind = rand() % enters.size();
    size_t enter_coord_1d = enters[enter_ind];
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, field.get_wid());
    x_ = enter_coord_2d.first;
    y_ = enter_coord_2d.second;
}

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
    path_ = path_finder.find_path(cur_ind, aim_ind_);
    std::cout << "path to reg office ind" << aim_ind_ << std::endl; 
    path_finder.print_path();
    std::cout << "cur coord = ( " << x_ << ", " << y_ << " )";

    printf("HERE!!\n");
    move();
}

//!! шагать нужно до того момента, пока мы не достигнем цели или не встанем в очередь (стоит проверять статус пассажира)
void Passenger::move()
{
    if (path_.empty()) {
        std::cout << "No path to move!" << std::endl;
        return;
    }
    printf("wow\n");
    std::cout << "cur coord = ( " << x_ << ", " << y_ << " )";
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    std::cout << "aim_ind = " << aim_ind_ << std::endl;

    while (status_ != WAITING_IN_LINE && cur_ind != aim_ind_)
    {
        std::cout << "cur coord = ( " << x_ << ", " << y_ << " )" << std::endl;
        std::cout << "cur_ind = " << cur_ind << std::endl;
        make_step();
        //size_t next_cell_ind = path_[1];
        //std::cout << "next_cell_ind = " << next_cell_ind << std::endl;
        //std::cout << "cur_ind = " << cur_ind << std::endl;
        //PathSituation step_status = field_.check_next_step(aim_ind_, cur_ind, next_cell_ind);
        //printf("pathsituation = %d\n", step_status);
        //std::pair<size_t, size_t> next_coord = f1dto2d(next_cell_ind, field_.get_wid());
        //std::cout << "next_coord = " << next_coord.first << ", " << next_coord.second << std::endl;

        //x_ = next_coord.first;
        //y_ = next_coord.second;
        std::cout << "new x y = " << x_ << ", " << y_ << std::endl;

        path_.erase(path_.begin());
        cur_ind = f2dto1d(x_, y_, field_.get_wid());
        std::cout << "cur_ind = " << cur_ind << std::endl;

        int a = 0;
        scanf("%d", &a);
    }
}

size_t Passenger::reg_proc()
{
    auto& gates = field_.get_gates();
    size_t gatenum = rand() % gates.size();
    RegOffice* the_office = static_cast<RegOffice*>(field_.get_cell_by_ind(aim_ind_));
    the_office->free_queue_space();
    return gates[gatenum]; 
}

void Passenger::end_algo()
{
    aim_ind_ = reg_proc();
    AStarPathFinder path_finder(field_);
    size_t cur_ind = f2dto1d(x_, y_, field_.get_wid());
    path_ = path_finder.find_path(cur_ind, aim_ind_);
    std::cout << "path to gate ind" << aim_ind_ << std::endl; 
    path_finder.print_path();
    move();
}

void Passenger::make_step()
{
    /*if (this == nullptr) {
        printf("CRITICAL: this is nullptr!\n");
        return;
    }
    
    printf("=== ENTER make_step() ===\n");
    printf("this address: %p\n", (void*)this);
    fflush(stdout);
    
    // ШАГ 1: Проверка field_
    printf("Checking field_...\n");
    fflush(stdout);
    
    try {
        printf("Field address: %p\n", (void*)&field_);
        size_t w = field_.get_wid();
        printf("Field width: %zu\n", w);
    } catch (...) {
        printf("ERROR: Cannot access field_\n");
        return;
    }

    printf("hello\n");

     printf("Checking path_...\n");
    printf("path_ address: %p\n", (void*)&path_);
    printf("path_ size: %zu\n", path_.size());
    fflush(stdout);

    if (path_.size() < 2) {
        std::cout << "ERROR: path too short in make_step()! Size = " 
                  << path_.size() << std::endl;
        return;
    }*/

    size_t next_cell_ind = path_[1];
    size_t cur_ind = path_[0];
    PathSituation step_status = field_.check_next_step(aim_ind_, cur_ind, next_cell_ind);
    printf("pathsituation = %d\n", step_status);
    std::pair<size_t, size_t> next_coord = f1dto2d(next_cell_ind, field_.get_wid());
    
    switch(step_status)
    {
        case PASSING_BY:
        case FREE:
        {
            printf("step\n");
            step(next_coord);
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

void Passenger::step(std::pair<size_t, size_t> next_coord)
{
    x_ = next_coord.first;
    y_ = next_coord.second;
}
    
void Passenger::get_in_line()
{
    set_status(WAITING_IN_LINE); //стоит ли делать так, чтобы он попробовал найти последнего в очереди? можно потом добавить
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

//-----------------------------------------------------------

Busy::Busy(Field& field) : Passenger(HIGH_SPEED, field) {}

//-----------------------------------------------------------

Old::Old(Field& field) : Passenger(LOW_SPEED, field) {}