#include "field.hpp"

Cell::Cell(size_t x, size_t y) : x_(x), y_(y) {}
Cell::Cell(size_t x, size_t y, bool passable) : x_(x), y_(y), passable_(passable) {} 

size_t Cell::get_x () const {return x_;}
size_t Cell::get_y () const {return y_;}
size_t Cell::get_next_psg_step() 
{
    if (!psg_)
        return ERROR_VAL;
    return psg_->get_next_step();
}

StatusTypes Cell::get_psg_status() 
{
    if (!psg_)
        return NULLPTR; 
    return psg_->get_status();
}
bool Cell::is_passable () const {return passable_;}

bool Cell::is_psg_nullptr()
{
    if (!psg_)
        return true;
    return false;
}

bool Cell::operator==(const Cell& other) const 
{
    return x_ == other.x_ && y_ == other.y_;
}

size_t Cell::get_psg_aim_ind() 
{
    if (!psg_)
        return ERROR_VAL;
    return psg_->get_aim();
}

bool Cell::is_occupied()
{
    if (!psg_)
        return false;
    return true;
}

void Cell::set_cell_free()
{
    psg_ = nullptr;
}

Passenger* Cell::get_psg()
{
    return psg_;
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

//-----------------------------------------------------------

size_t Field::get_heig () const {return heig_;}
size_t Field::get_wid () const {return wid_;}
const std::vector<size_t>& Field::get_reg_offices () const {return reg_offices_;}
const std::vector<size_t>& Field::get_enterances () const {return enterances_;}
const std::vector<size_t>& Field::get_gates () const {return gates_;}
const std::vector<size_t>& Field::get_obstacles () const {return obstacles_;}

Cell* Field::get_cell_by_ind(size_t ind) const 
{
    if (ind < cells_.size()) {
        return cells_[ind].get();
    }
    return nullptr;
}

bool Field::is_passable(size_t ind)
{
    auto cell_it = std::find(obstacles_.begin(), obstacles_.end(), ind);
    if (cell_it != obstacles_.end())
        return false;
    return true;
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

    for (size_t i = 0; i < wid_*heig_; i++)
    {
        std::pair<size_t, size_t> coords = f1dto2d(i, wid_);
        size_t x = coords.first;
        size_t y = coords.second;

        switch(field_prototipe[i])
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

size_t Field::find_free_reg_office_ind()
{
    return *std::min_element(reg_offices_.begin(), reg_offices_.end(), [this](size_t a_ind, size_t b_ind)
    {
        RegOffice* a_office = static_cast<RegOffice*>(cells_[a_ind].get());
        RegOffice* b_office = static_cast<RegOffice*>(cells_[b_ind].get());
        return a_office->get_queue_len() < b_office->get_queue_len();
    });
}


//void Field::draw () const
//{
//    for (size_t y = 0; y < heig_; y++)
//    {
//        for (size_t x = 0; x < wid_; x++)
//        {
//            size_t ind =  f2dto1d(x, y, wid_);
//            cells_[ind]->draw();
//        }
//        std::cout << std::endl;
//    }
//}

PathSituation Field::check_next_step(size_t psg_aim, size_t psg_cur_ind, size_t next_step_ind)
{
    Cell* cell = get_cell_by_ind(next_step_ind);
    if (!cell)
        return NO_CELL_NULLPTR;

    if (!cell->is_occupied())
    {
        if (!is_there_others_interested(next_step_ind))
            return FREE;
        return OTHER_INTERESTED;
    }
    
    bool is_psg_nullptr = cell->is_psg_nullptr();
    if (!is_psg_nullptr)
    {
        StatusTypes opponent_status = cell->get_psg_status();
        if (opponent_status == WAITING_IN_LINE)
        {
            size_t opponents_aim = cell->get_psg_aim_ind();
            if (opponents_aim == psg_aim)
                return QUEUE;
            return QUEUE_OBSTACLE; 
        }
        size_t opponent_next_step_ind = cell->get_next_psg_step();
        if (psg_cur_ind == opponent_next_step_ind)
            return COLLISION;
    }

    return PASSING_BY;
}

bool Field::is_there_others_interested(size_t next_step_ind)
{
    std::pair<int, int> cur_coord = f1dto2d(next_step_ind, wid_);
    for (auto& step : steps)
    {
        std::pair<int, int> new_coord = {cur_coord.first + step.first, cur_coord.second + step.second};
        //std::cout << "new step " << new_coord.first << ", " << new_coord.second << std::endl;
        if (new_coord.first < 0 || new_coord.second < 0)
        {
            //std::cout << "less" << std::endl;
            continue;
        }
        if (new_coord.first > static_cast<int>(wid_) || new_coord.second > static_cast<int>(heig_))
        {
            //std::cout << "more" << std::endl;
            continue;
        }
        size_t cell_ind = f2dto1d(new_coord.first, new_coord.second, wid_);
        Cell* cell = get_cell_by_ind(cell_ind);
        
        size_t opponents_aim = cell->get_psg_aim_ind();
        if (opponents_aim == ERROR_VAL)
            continue;           //здесь просто нет пассажира
        if (opponents_aim == next_step_ind)
            return true;
    }
    return false;
}

std::vector<size_t> Field::update_obstacles_by_opponent(size_t opponents_coord)
{
    std::vector<size_t> obstacles;
    std::copy(obstacles_.begin(), obstacles_.end(), obstacles.begin());
    obstacles.push_back(opponents_coord);
    return obstacles;
}

std::vector<size_t> Field::update_obstacles_by_queues()
{
    std::vector<size_t> obstacles;
    Cell* cell;
    for (size_t ind = 0 ; ind < wid_ * heig_; ind++)
    {
        cell = get_cell_by_ind(ind);
        if (!cell->is_passable() || cell->is_occupied())
            obstacles.push_back(ind);
    }
    return obstacles;
}