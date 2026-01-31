#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>

class DrawableObject
{
public:
    virtual void draw() const = 0;

    virtual ~DrawableObject() = default;
};

//-------------------------------------------------------------------------------------------------

class Cell : DrawableObject
{
protected:

    size_t x_ = 0;
    size_t y_ = 0;
    bool passable_ = true;

public:

    Cell() = default;

    Cell(size_t x, size_t y) : x_(x), y_(y) {}

    Cell(size_t x, size_t y, bool passable) : x_(x), y_(y), passable_(passable) {} 

    size_t get_x () const {return x_;}
    size_t get_y () const {return y_;}
    bool is_passable () const {return passable_;}

    bool operator==(const Cell& other) const 
    {
        return x_ == other.x_ && y_ == other.y_;
    }

    void draw () const override
    {
        if (passable_)
            std::cout << ".";
        else
            std::cout << "#";
    }
};

class Gate : public Cell
{
    size_t gatenum_ = 0;

public:

    Gate(size_t x, size_t y, size_t gatenum) : Cell(x, y), gatenum_(gatenum) {}

    size_t get_gatenum () const {return gatenum_;}

    void draw () const override
    {
        std::cout << "{}";
    }
};

class Enterance : public Cell
{
    size_t enternum_ = 0;

public:

    Enterance(size_t x, size_t y, size_t enternum) : Cell(x, y), enternum_(enternum) {}

    size_t get_enternum () const {return enternum_;}

    void draw () const override
    {
        std::cout << "_";
    }
};

class RegOffice : public Cell
{
    size_t queue_len_ = 0;

public:

    RegOffice(size_t x, size_t y) : Cell(x, y, true) {}

    size_t reg_proc(Field& field)
    {
        auto& gates = field.get_gates();
        size_t gatenum = rand() % (gates.size() + 1);
        queue_len_--;
        return gates[gatenum]; 
    }

    void take_turn()
    {
        queue_len_++;
    }

    size_t get_queue_len () const {return queue_len_;}

    bool operator<(const RegOffice& other) const 
    {
        return queue_len_ < other.queue_len_;
    }

    void draw () const override
    {
        std::cout << "$";
    }
};

//------------------------------------------------------------------------------------------

const size_t MID_SPEED = 50;
const size_t HIGH_SPEED = 100;
const size_t LOW_SPEED = 10;

class Passenger
{
protected:

    size_t speed_ = MID_SPEED;
    size_t aim_ind_;
    std::vector<size_t> path_;
    
public:

    Passenger() {}
    Passenger(size_t speed) : speed_(speed) {}
 
    const size_t get_aim () const {return aim_ind_;}
    const std::vector<size_t>& get_path () const {return path_;}

    void set_aim(size_t aim)
    {
        aim_ind_ = aim;
    }

    void start_algo(Field& field)
    {
        size_t free_office_ind = field.find_free_reg_office_ind();
        RegOffice* free_office = static_cast<RegOffice*>(field.get_cell_by_ind(free_office_ind));
        free_office->take_turn();
        aim_ind_ = free_office_ind;
        path_ = find_path();
    }

    std::vector<size_t> find_path()
    {
        //astar_algo_for_aim_...
    }

    void end_algo(Field& field, RegOffice& reg_office)
    {
        aim_ind_ = reg_office.reg_proc(field);
        path_ = find_path();
    }
};

class Busy : public Passenger
{
public:
    Busy() : Passenger(HIGH_SPEED) {}
};

class Old : public Passenger
{
public:
    Old() : Passenger(LOW_SPEED) {}
};

//--------------------------------------------------------------------------------------

enum CellType
{
    SIMPLE,
    OBSTACLE,
    GATE,
    OFFICE,
    ENTER
};

size_t f2dto1d (size_t x, size_t y, size_t wid)
{
    return y * wid + x;
}

std::pair<size_t, size_t> f1dto2d (size_t ind, size_t wid)
{
    size_t x = ind % wid;
    size_t y = ind / wid;

    return {x, y};
}

class Field
{
    size_t len_ = 0;
    size_t wid_ = 0;
    std::vector<std::unique_ptr<Cell>> cells_;  

    std::vector<size_t> reg_offices_;
    std::vector<size_t> enterances_;
    std::vector<size_t> gates_;
    std::vector<size_t> obstacles_;

public:

    size_t get_len () const {return len_;}
    size_t get_wid () const {return wid_;}
    // std::vector<Cell*> get_cells () const {return cells_;}
    const std::vector<size_t>& get_reg_offices () const {return reg_offices_;}
    const std::vector<size_t>& get_enterances () const {return enterances_;}
    const std::vector<size_t>& get_gates () const {return gates_;}

    Field(size_t len, size_t wid, 
          const std::vector<std::pair<size_t, size_t>>& obstacles,
          const std::vector<std::pair<size_t, size_t>>& office_pos,
          const std::vector<std::pair<size_t, size_t>>& entrance_pos,
          const std::vector<std::pair<size_t, size_t>>& gate_pos) : len_(len), wid_(wid)
    {
        std::vector<CellType> field_prototipe(wid_*len_, SIMPLE); 
        for (const auto& obstacle : obstacles)
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

    size_t find_free_reg_office_ind()
    {
        return *std::min_element(reg_offices_.begin(), reg_offices_.end(), [this](size_t a_ind, size_t b_ind)
        {
            RegOffice* a_office = static_cast<RegOffice*>(cells_[a_ind].get());
            RegOffice* b_office = static_cast<RegOffice*>(cells_[b_ind].get());

            return a_office->get_queue_len() < b_office->get_queue_len();
        });
    }

    Cell* get_cell_by_ind(size_t ind) const 
    {
        if (ind < cells_.size()) {
            return cells_[ind].get();
        }
        return nullptr;
    }
};