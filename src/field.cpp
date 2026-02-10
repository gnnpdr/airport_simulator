#include "field.hpp"

Cell::Cell(size_t x, size_t y) : x_(x), y_(y) {}
Cell::Cell(size_t x, size_t y, bool passable) : x_(x), y_(y),  passable_(passable) {} 

bool Cell::is_passable () const {return passable_;}

bool Cell::operator==(const Cell& other) const 
{
    return x_ == other.x_ && y_ == other.y_;
}

 bool Cell::is_occupied() const 
 {
    return !psg_.expired();
}

void Cell::set_cell_free()
{
    psg_.reset();
}

std::shared_ptr<Passenger> Cell::get_psg() const 
{
        return psg_.lock();
}

void Cell::set_psg(std::shared_ptr<Passenger> psg) 
{
    psg_ = psg;
}

size_t Cell::get_next_psg_step() 
{
    if (auto psg = psg_.lock()) 
        return psg->get_next_step();

    return ERROR_VAL;
}

StatusTypes Cell::get_psg_status() 
{
    if (auto psg = psg_.lock())
        return psg->get_status();
    return NULLPTR;
}

size_t Cell::get_psg_aim_ind() 
{
    if (auto psg = psg_.lock())
        return psg->get_aim();
    return ERROR_VAL;
}

//-----------------------------------------------------------

Gate::Gate(size_t x, size_t y, size_t gatenum) : Cell(x, y), gatenum_(gatenum) {}

size_t Gate::get_gatenum () const {return gatenum_;}

//-----------------------------------------------------------

Enterance::Enterance(size_t x, size_t y, size_t enternum) : Cell(x, y), enternum_(enternum) {}

size_t Enterance::get_enternum () const {return enternum_;}

//-----------------------------------------------------------

RegOffice::RegOffice(size_t x, size_t y) : Cell(x, y, true) {}

void RegOffice::take_turn()
{
    queue_len_++;
}

void RegOffice::free_queue_space()
{
    queue_len_--;
}

size_t RegOffice::get_queue_len () const {return queue_len_;}

bool RegOffice::operator<(const RegOffice& other) const 
{
    return queue_len_ < other.queue_len_;
}


//------------------------------------------------------------------------------------------------------------------------------------------------------------------------

size_t Field::get_heig () const {return heig_;}
size_t Field::get_wid () const {return wid_;}

std::vector<size_t>& Field::get_obstacles()
{
    return obstacles_;
}

Cell* Field::get_cell_by_ind(size_t ind) const 
{
    if (ind < cells_.size()) {
        return cells_[ind].get();
    }
    return nullptr;
}

Field::Field(size_t heig, size_t wid, 
      const std::vector<std::pair<size_t, size_t>>& obstacles_pos,
      const std::vector<std::pair<size_t, size_t>>& office_pos,
      const std::vector<std::pair<size_t, size_t>>& entrance_pos,
      const std::vector<std::pair<size_t, size_t>>& gate_pos) : heig_(heig), wid_(wid)
{
    std::vector<CellType> field_prototipe(wid_*heig_, SIMPLE); 
    for (const auto& obstacle : obstacles_pos)
    {
        int index = f2dto1d(obstacle.first, obstacle.second, wid_);
        field_prototipe[index] = OBSTACLE;
        obstacles_.push_back(index);
    }
    for (const auto& office : office_pos)
    {
        int index = f2dto1d(office.first, office.second, wid_);
        field_prototipe[index] = OFFICE;
        reg_offices_.push_back(index);
    }
    for (const auto& enter : entrance_pos)
    {
        int index = f2dto1d(enter.first, enter.second, wid_);
        field_prototipe[index] = ENTER;
        enterances_.push_back(index);
    }
    for (const auto& gate : gate_pos)
    {
        int index = f2dto1d(gate.first, gate.second, wid_);
        field_prototipe[index] = GATE;
        gates_.push_back(index);
    }

    size_t enter_cnt = 0;
    size_t gate_cnt = 0;

    for (size_t ind = 0; ind < wid_*heig_; ind++)
    {
        auto coord = f1dto2d(ind, wid_);
        size_t x = coord.first;
        size_t y = coord.second;
        switch(field_prototipe[ind])
        {
            case SIMPLE:
            {
                std::unique_ptr<Cell> cell = std::make_unique<Cell>(x, y, true);
                cells_.push_back(std::move(cell));
                break;
            }
            case OBSTACLE:
            {
                std::unique_ptr<Cell> cell = std::make_unique<Cell>(x, y, false);
                cells_.push_back(std::move(cell));
                break;  
            }
            case OFFICE:
            {
                std::unique_ptr<Cell> cell = std::make_unique<RegOffice>(x, y);
                cells_.push_back(std::move(cell));
                break;
            }
            case ENTER:
            {
                std::unique_ptr<Cell> cell = std::make_unique<Enterance>(x, y, enter_cnt);
                enter_cnt++;
                cells_.push_back(std::move(cell));
                break;
            }
            case GATE:
            {
                std::unique_ptr<Cell> cell = std::make_unique<Gate>(x, y, gate_cnt);
                gate_cnt++;
                cells_.push_back(std::move(cell));
                break;
            }
        }
    }
}

//---------------------------------------------------------------

std::shared_ptr<Passenger> Field::add_passenger() 
{
    auto new_passenger = std::make_shared<Passenger>();
    
    size_t enter_ind = rand() % enterances_.size();
    size_t enter_coord_1d = enterances_[enter_ind];
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, wid_);
    
    new_passenger->set_x(enter_coord_2d.first);
    new_passenger->set_y(enter_coord_2d.second);
    
    Cell* cell = get_cell_by_ind(enter_coord_1d);
    cell->set_psg(new_passenger);
    
    passengers_.push_back(new_passenger);
    
    return new_passenger;
}


//-----------------------------------

//сначала проходим по всем пассажирам и простраиваем путь тем, кому нужно - состояния - FIND_REG и REG
//!в это время сразу меняется состояние и они могут сразу участвовать в планировании маршрутов других, это нормально
//!!причем тем, кто выбрал регистратуру не нужно занимать очередь в момент, когда они только выдвигаются, пусть они занимают очередь, когда становятся в конец очереди
//!!или достигают регистратуры, иначе опять получится изменение состояния поля между планированием и шагом
//затем проходимся по тем, кто планирует шагать - ROING_TO_REG, WAITING (им же нужно двигаться в очереди), GOING_TO_GATE
//затем все единоразово шагаем
void Field::update(float delta_time)
{
    printf("update\n");
    if (is_paused_) return;

    simulation_time_ += delta_time;

    set_paths();

    reset_statuses();
    solve_collisions();
    plan_steps();

    step();
}

void Field::set_paths()
{
    printf("set paths\n");
    for (auto& psg : passengers_)
    {
        StatusTypes status = psg->get_status();
        printf("status %d\n", status);
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
        }
        else if (status == REG)
        {
            size_t reg_office_ind = psg->get_aim();
            RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(reg_office_ind));
            reg_office->free_queue_space();
            
            aim_ind = reg();
            psg->set_aim(aim_ind);
            psg->set_status(GOING_TO_GATE);
        }

        AStarPathFinder path_finder(*this);
        auto path = path_finder.find_path(cur_ind, aim_ind);

        psg->set_path(path);

        path_finder.print_path();
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

            if (!neighbor_cell) continue;

            size_t opponents_aim = neighbor_cell->get_psg_aim_ind();
            if (opponents_aim == ERROR_VAL)
                continue;           //здесь просто нет пассажира
            if (opponents_aim == next_step_ind)
                interested.push_back(cell_ind);
        }

        if (interested.empty())
            continue;
    
        size_t walker_ind = interested[rand() % interested.size()];
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
        printf("status %d\n", psg->get_status());
        if (psg->get_step_status() != RESET)
            continue;

        auto& path = psg->get_path();

        if (path.size() == 1)
        {
            end_of_path(psg);
            return;
        }

        size_t aim_ind = psg->get_aim();
        PathSituation step_status = check_next_step(aim_ind, path[0], path[1]);
        psg->set_step_status(step_status);
    }
}

//-------------------------------------------------------

PathSituation Field::check_next_step(size_t psg_aim, size_t psg_cur_ind, size_t next_step_ind)
{
    Cell* cell = get_cell_by_ind(next_step_ind);
    if (!cell)
        return NO_CELL_NULLPTR;

    if (!cell->is_occupied())
    {
        return FREE;
    }
    
    bool is_occupied = cell->is_occupied();
    if (!is_occupied)
    {
        StatusTypes opponent_status = cell->get_psg_status();
        if (opponent_status == WAITING)
        {
            size_t opponents_aim = cell->get_psg_aim_ind();
            if (opponents_aim == psg_aim)
                return QUEUE;
            return QUEUE_OBSTACLE; 
        }
    }

    return PASSING_BY;
}

void Field::step()
{
    for (auto& psg : passengers_)
    {
        PathSituation step_status = psg->get_step_status();

        switch(step_status)
        {
            case PASSING_BY:
            case FREE:
            case COLLISION_GO:
            {
                move_psg(psg);
                break;
            }
            case COLLISION_WAIT:
            {
                break;
            }
            case QUEUE:
            {
                set_to_line(psg);
                break;
            }
            case QUEUE_OBSTACLE:
            {
                get_around_queue(psg);
            }
            case RESET:
            case NO_CELL_NULLPTR:
                break;
        }
    }       
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
    Cell* next_cell = get_cell_by_ind(path[1]);

    next_cell->set_psg(psg);
    cur_cell->set_cell_free();
    auto next_coord = f1dto2d(path[1], wid_);
    psg->set_x(next_coord.first);
    psg->set_y(next_coord.second);
    path.erase(path.begin());
    psg->set_path(path);
}


void Field::set_to_line(std::shared_ptr<Passenger> psg)
{
    size_t reg_office_ind = psg->get_aim();
    RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(reg_office_ind));
    reg_office->take_turn();
    psg->set_status(WAITING);
}

void Field::get_around_queue(std::shared_ptr<Passenger> psg)
{
    size_t cur_x = psg->get_x();
    size_t cur_y = psg->get_y();
    size_t cur_ind = f2dto1d(cur_x, cur_y, wid_);
    size_t aim_reg_office_ind = psg->get_aim();
    std::vector<size_t> new_obstacles = update_obstacles_by_queues(aim_reg_office_ind);
    AStarPathFinder path_finder(*this, new_obstacles);
    std::vector<size_t> updated_path = path_finder.find_path(cur_ind, aim_reg_office_ind);
}

//---------------------------------------

size_t Field::find_free_reg_office_ind()
{
    return *std::min_element(reg_offices_.begin(), reg_offices_.end(), [this](size_t a_ind, size_t b_ind)
    {
        RegOffice* a_office = static_cast<RegOffice*>(cells_[a_ind].get());
        RegOffice* b_office = static_cast<RegOffice*>(cells_[b_ind].get());
        return a_office->get_queue_len() < b_office->get_queue_len();
    });
}

std::vector<size_t> Field::update_obstacles_by_queues(size_t aim_reg_office_ind)
{
    std::vector<size_t> obstacles;
    std::copy(obstacles_.begin(), obstacles_.end(), obstacles.begin());

    //проходимся по всем пассажирам и берем индесы тех, кто стоит не в нашу стойку, они нам мешают
    for (auto& psg : passengers_)
    {
        StatusTypes psg_status = psg->get_status();

        if (psg_status != WAITING)
            continue;

        size_t psg_aim = psg->get_aim();

        if (psg_aim != aim_reg_office_ind)
        {
            size_t cur_x = psg->get_x();
            size_t cur_y = psg->get_y();
            size_t psg_ind = f2dto1d(cur_x, cur_y, wid_);
            obstacles.push_back(psg_ind);
        }
    }

    return obstacles;
}

void Field::end_of_path(std::shared_ptr<Passenger> psg)
{
    StatusTypes psg_status = psg->get_status(); 
    if (psg_status == GOING_TO_REG || psg_status == WAITING)
    {
        if(psg_status == GOING_TO_REG)
        {
            size_t aim_reg_office_ind = psg->get_aim();
            RegOffice* reg_office = static_cast<RegOffice*>(get_cell_by_ind(aim_reg_office_ind));
            reg_office->take_turn();
        }

        psg->set_status(REG);
    }
    if (psg_status == GOING_TO_GATE)
    {
        remove_psg(psg);
    }
}


void Field::remove_psg(std::shared_ptr<Passenger> psg)
{
    if (!psg)   
        return;

    size_t cur_x = psg->get_x();
    size_t cur_y = psg->get_y();
    size_t psg_ind = f2dto1d(cur_x, cur_y, wid_);
    Cell* cell = get_cell_by_ind(psg_ind);
    if (cell)
        cell->set_cell_free();

    auto it = std::find(passengers_.begin(), passengers_.end(), psg);
    if (it != passengers_.end()) 
    {
        // Удаляем из вектора - shared_ptr автоматически уничтожит объект
        // когда на него не останется сильных ссылок
        passengers_.erase(it);
    } else
        std::cout << "Psg not found" << std::endl;
}

//-----------------------------------------------------

//мб убрать

std::vector<size_t> Field::update_obstacles_by_opponent(size_t opponents_coord)
{
    std::vector<size_t> obstacles;
    std::copy(obstacles_.begin(), obstacles_.end(), obstacles.begin());
    obstacles.push_back(opponents_coord);
    return obstacles;
}