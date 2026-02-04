#include "common.hpp"
#include "field.hpp"

Cell::Cell(size_t x, size_t y) : x_(x), y_(y) {}

Cell::Cell(size_t x, size_t y, bool passable) : x_(x), y_(y), passable_(passable) {} 

size_t Cell::get_x () const {return x_;}
size_t Cell::get_y () const {return y_;}
bool Cell::is_passable () const {return passable_;}

bool Cell::operator==(const Cell& other) const 
{
    return x_ == other.x_ && y_ == other.y_;
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

size_t Field::get_len () const {return len_;}
size_t Field::get_wid () const {return wid_;}
const std::vector<size_t>& Field::get_reg_offices () const {return reg_offices_;}
const std::vector<size_t>& Field::get_enterances () const {return enterances_;}
const std::vector<size_t>& Field::get_gates () const {return gates_;}
const std::vector<size_t>& Field::get_obstacles () const {return obstacles_;}

Field::Field(size_t len, size_t wid, 
      const std::vector<std::pair<size_t, size_t>>& obstacles_pos,
      const std::vector<std::pair<size_t, size_t>>& office_pos,
      const std::vector<std::pair<size_t, size_t>>& entrance_pos,
      const std::vector<std::pair<size_t, size_t>>& gate_pos) : len_(len), wid_(wid)
{
    std::vector<CellType> field_prototipe(wid_*len_, SIMPLE); 
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

    for (size_t i = 0; i < wid_*len_; i++)
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

void Field::draw () const
{
    for (size_t y = 0; y < len_; y++)
    {
        for (size_t x = 0; x < wid_; x++)
        {
            size_t ind =  f2dto1d(x, y, wid_);
            cells_[ind]->draw();
        }
        std::cout << std::endl;
    }
}

size_t Cell::get_next_psg_step() {return psg_->get_next_step();}

StatusTypes Cell::get_psg_status() {return psg_->get_status();}