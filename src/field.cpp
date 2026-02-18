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
    std::cout << "Clearing cell at (" << x_ << "," << y_ << ")" << std::endl;
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

bool RegOffice::operator<(const RegOffice& other) const 
{
    return queue_.size() < other.queue_.size();
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
    if (ind >= cells_.size()) {
        std::cout << "[ERROR] get_cell_by_ind: index " << ind 
                  << " out of bounds (max=" << cells_.size() - 1 << ")" << std::endl;
        return nullptr;
    }
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

    //std::copy(enterances_.begin(), enterances_.end(), free_enterances_.begin());
    free_enterances_ = enterances_;

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

std::shared_ptr<Passenger> Field::add_passenger(size_t num) 
{
    auto new_passenger = std::make_shared<Passenger>(num);
    entrance_queue_.push_back(new_passenger);

    std::cout << "add psg to the queue, queue len is " << entrance_queue_.size() << std::endl;
    
    return new_passenger;
}

void Field::process_entrance_queue() 
{
    if (entrance_queue_.empty()) 
        return;
    
    auto new_passenger = entrance_queue_.front();
    if (!free_enterances_.size())
        return;

    entrance_queue_.pop_front();

    size_t rand_ind = rand() % free_enterances_.size();
    size_t enter_coord_1d = free_enterances_[rand_ind];

    //std::cout << "rand_ind " << rand_ind << " free_enter_ind " << enter_coord_1d << std::endl;
    auto it = free_enterances_.begin() + rand_ind;
    free_enterances_.erase(it);
    std::pair<size_t, size_t> enter_coord_2d = f1dto2d(enter_coord_1d, wid_);

    std::cout << "new psg added to the coord " << enter_coord_2d.first << " / " << enter_coord_2d.second << std::endl;
    
    new_passenger->set_x(enter_coord_2d.first);
    new_passenger->set_y(enter_coord_2d.second);
    
    Cell* cell = get_cell_by_ind(enter_coord_1d);

    cell->set_psg(new_passenger);
    
    passengers_.push_back(new_passenger);

}

//-----------------------------------



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
    obstacles.reserve(obstacles_.size() + reg_offices_.size());
    obstacles.insert(obstacles.end(), obstacles_.begin(), obstacles_.end());
    obstacles.insert(obstacles.end(), reg_offices_.begin(), reg_offices_.end());
    obstacles.erase(std::remove(obstacles.begin(), obstacles.end(), aim_reg_office_ind), obstacles.end());
    obstacles_ = obstacles;

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