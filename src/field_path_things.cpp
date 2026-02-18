#include "field.hpp"

void Field::update(float delta_time)
{
    if (is_paused_) 
        return;

    simulation_time_ += delta_time;

    set_paths();

    reset_statuses();
    solve_collisions();
    plan_steps();

    step();

    process_entrance_queue();
}

void Field::set_paths()
{
    printf("set paths\n");
    for (auto& psg : passengers_)
    {
        StatusTypes status = psg->get_status();
        printf("num %d status %d\n", psg->get_num(), psg->get_status());
        if (status != FIND_REG && status != REG)
            continue;
        
        size_t cur_x = psg->get_x();
        size_t cur_y = psg->get_y();
        size_t cur_ind = f2dto1d(cur_x, cur_y, wid_);
        size_t aim_ind = 0;
        if (status == FIND_REG)
        {
            aim_ind = find_free_reg_office_ind();
            psg->set_aim(aim_ind);
            psg->set_status(GOING_TO_REG);

            AStarPathFinder path_finder(aim_ind, cur_ind, *this);
            auto path = path_finder.find_path();
            psg->set_path(path);
            path_finder.print_path();
        }
        else if (status == REG)
        {
            size_t reg_office_ind = psg->get_aim();
            RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(reg_office_ind));

            if (reg_office->get_service_time() <= 0) 
            {
                reg_office->free_queue_space();
                aim_ind = reg();
                psg->set_aim(aim_ind);
                psg->set_status(GOING_TO_GATE);

                AStarPathFinder path_finder(aim_ind, cur_ind, *this);                         //надо покрасивее сделать. в любом, случае, если это кнец, нужно им еще и путь дать
                auto path = path_finder.find_path();
                psg->set_path(path);
                path_finder.print_path();
            }
        }
    }
}

void Field::reset_statuses()
{
    printf("reset statuses\n");
    for (auto& psg : passengers_)
    {
        psg->set_step_status(RESET);
    }
}

void Field::solve_collisions()
{
    printf("solve collisions\n");

    for (size_t ind = 0; ind < wid_ * heig_; ind++)
    {
        Cell* cell = get_cell_by_ind(ind);
        if (!cell || !cell->is_occupied())
            continue;

        size_t next_step_ind = cell->get_next_psg_step();
        std::pair<int, int> next_step_coord = f1dto2d(next_step_ind, wid_);
        std::vector<size_t> interested;
        for (auto& step : steps)
        {
            std::pair<int, int> new_coord = {next_step_coord.first + step.first, next_step_coord.second + step.second};
            if (new_coord.first < 0 || new_coord.second < 0 || new_coord.first >= static_cast<int>(wid_) || new_coord.second >= static_cast<int>(heig_))
                continue;

            size_t cell_ind = f2dto1d(new_coord.first, new_coord.second, wid_);

            Cell* neighbor_cell = get_cell_by_ind(cell_ind);

            if (!neighbor_cell || !neighbor_cell->is_occupied()) 
                continue;

            size_t opponents_next_step = neighbor_cell->get_next_psg_step();
            if (opponents_next_step == next_step_ind)
                interested.push_back(cell_ind);
        }

        if (interested.empty())
            continue;
    
        size_t walker_ind = interested[rand() % interested.size()];

        printf("interested\n");
        for (int i = 0; i < interested.size(); i++)
        {
            printf("%d ", interested[i]);
        }
        printf("\n");

        for (size_t cell_ind : interested)
        {
            Cell* conflict_cell = get_cell_by_ind(ind);
            if (!conflict_cell) 
                continue;
            auto psg = cell->get_psg();
            if (!psg) 
                continue;

            if (cell_ind == walker_ind)
            {
                psg->set_step_status(COLLISION_GO);
                printf("collision go on psg with cell %d %d\n", psg->get_x(), psg->get_y());
            }
            else
                psg->set_step_status(COLLISION_WAIT);
        }
    }
}   

void Field::plan_steps()
{
    printf("plan steps\n");
    for (auto& psg : passengers_)
    {
        printf("num %d status %d, step status %d\n", psg->get_num(), psg->get_status(), psg->get_step_status());
        auto status = psg->get_status();

        if (psg->get_step_status() != RESET)
            continue;

        auto& path = psg->get_path();
        printf("path_size %d\n", path.size());

        if (path.size() == 1 && status == REG)
        {
            printf("1, going to reg\n");
            psg->set_step_status(REALLY_REG);
            continue;
        }
        if (path.size() == 2 && status == GOING_TO_REG) 
        {
            printf("2, going to reg\n");
            size_t aim_reg_office_ind = psg->get_aim();
            RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(aim_reg_office_ind));
            if (reg_office->is_occupied()) 
            {
                StatusTypes reging_psg_status = reg_office->get_psg_status();
                if (reging_psg_status == REG)
                {
                    printf("queue\n");
                    psg->set_step_status(QUEUE);
                    continue;
                }
            }
            //здесь может возникнуть проблема, если пассажир, которыый сечас регистрируется, не сможет уйти с клетки регистрации. 
            //лучше изменить механику очереди и сделать так, чтобы обычный пассажиры не могли просто так вставать в клетку регистрации
            //и те, кто зарегистрировался, сразу уходили
            printf("free for reg\n");
            psg->set_step_status(START_REG);        //нужен ли этот статус или можно обозначить как просто free?

            continue;
        }

        if (path.size() == 1 && status == GOING_TO_GATE)
        {
            remove_psg(psg);
            continue;
        }

        size_t aim_ind = psg->get_aim();
        PathSituation step_status = check_next_step(aim_ind, path[0], path[1]);
        psg->set_step_status(step_status);
    }
}



PathSituation Field::check_next_step(size_t psg_aim, size_t psg_cur_ind, size_t next_step_ind)
{
    Cell* cell = get_cell_by_ind(next_step_ind);
    if (!cell)
        return NO_CELL_NULLPTR;

    std::cout << "Checking cell " << next_step_ind 
              << " occupied=" << cell->is_occupied();
    
    if (cell->is_occupied()) {
        auto occupant = cell->get_psg();
        if (occupant) {
            std::cout << " by passenger " << occupant->get_num() 
                      << " status " << occupant->get_status();
        }
    }

    if (!cell->is_occupied())
    {
        return FREE;
    }
    
    StatusTypes opponent_status = cell->get_psg_status();
    if (opponent_status == WAITING)
    {
        size_t opponents_aim = cell->get_psg_aim_ind();
        if (opponents_aim == psg_aim)
            return QUEUE;
        return QUEUE_OBSTACLE; 
    }

    if (opponent_status == REG)
    {
        return NO_MOVING;
    }

    if (opponent_status == GOING_TO_REG || opponent_status == GOING_TO_GATE)
    {
        return PASSING_BY;
    }

    return NO_MOVING;
}

void Field::step()
{
    for (auto& psg : passengers_)
    {
        PathSituation step_status = psg->get_step_status();
        std::cout << "num " << psg->get_num() << " step status " << step_status << std::endl;

        switch(step_status)
        {
            case COLLISION_GO:
                printf("COLLISION GO!\n");
            case PASSING_BY:
                printf("PASSING\n");
            case FREE:
            {
                printf("STEP!\n");
                move_psg(psg);
                break;
            }
            case COLLISION_WAIT:
            {
                break;
            }
            case QUEUE:
            {
                //set to line бесполезная функция
                size_t reg_office_ind = psg->get_aim();
                RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(reg_office_ind));
                reg_office->take_turn(psg);
                break;
            }
            case QUEUE_OBSTACLE:
            {
                get_around_queue(psg);
            }
            case RESET:
            {
                printf("ERROR OCHEV\n");
                break;
            }
            case REALLY_REG:
            {
                size_t aim_reg_office_ind = psg->get_aim();
                RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(aim_reg_office_ind));
                std::cout << "reg is going" << std::endl;
                reg_office->update();
                break;
            }
            case START_REG:
            {
                printf("START REG\n");
                move_psg(psg);
                size_t aim_reg_office_ind = psg->get_aim();
                RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(aim_reg_office_ind));
                reg_office->take_turn(psg);
                break;
            }
            case NO_MOVING:
            case NO_CELL_NULLPTR:
            {
                printf("WAITING\n");
                break;
            }       
        }
    }       
}

//для тех, кто зашел на слетку меняется статус на регистрацию и устанавливается время регистрации
//для иех, кто еще не зашел на клетку, ставится статус ожидания и они занимаю место в очереди
void RegOffice::take_turn(std::shared_ptr<Passenger> psg) 
{
    if (x_ == psg->get_x() && y_ == psg->get_y())            //какой ужас
    {
        psg->set_status(REG);
        service_time_ = SERVICE_DURATION;
        return;
    }

    psg->set_status(WAITING);
    add_to_queue(psg);
}

//-----------------------------------------------

size_t Field::reg()
{   
    size_t gatenum = rand() % gates_.size();
    return gates_[gatenum]; 
}

void Field::move_psg(std::shared_ptr<Passenger> psg)
{
    auto path = psg->get_path();
    Cell* cur_cell = get_cell_by_ind(path[0]);
    
    //если уходит со входа, то возвращаем вход в список свободных
    //проверять на то, нет ли уже этой клетки там не надо, мы же ее точно заняли ранее...
    auto it = std::find(enterances_.begin(), enterances_.end(), path[0]);
    if (it != enterances_.end())
    {
        free_enterances_.push_back(path[0]);
    }

    Cell* next_cell = get_cell_by_ind(path[1]);

    next_cell->set_psg(psg);
    cur_cell->set_cell_free();
    auto next_coord = f1dto2d(path[1], wid_);
    psg->set_x(next_coord.first);
    psg->set_y(next_coord.second);

    std::cout << "new coord is " << next_coord.first << " / " << next_coord.second << std::endl;
    std::cout << "Cell " << path[1] << " now occupied by passenger " 
          << psg->get_num() << " is cell occupied status " << next_cell->is_occupied() << std::endl;
    path.erase(path.begin());
    psg->set_path(path);
}


void Field::set_to_line(std::shared_ptr<Passenger> psg)
{
    size_t reg_office_ind = psg->get_aim();
    RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(reg_office_ind));
    reg_office->take_turn(psg);
}

void Field::get_around_queue(std::shared_ptr<Passenger> psg)
{
    size_t cur_x = psg->get_x();
    size_t cur_y = psg->get_y();
    size_t cur_ind = f2dto1d(cur_x, cur_y, wid_);
    size_t aim_reg_office_ind = psg->get_aim();
    std::vector<size_t> new_obstacles = update_obstacles_by_queues(aim_reg_office_ind);
    AStarPathFinder path_finder(aim_reg_office_ind, cur_ind, *this, new_obstacles);
    std::vector<size_t> updated_path = path_finder.find_path();
}