#pragma once
#include "psg.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>

class Passenger;

class DrawableObject
{
public:
    virtual void draw() const = 0;

    virtual ~DrawableObject() = default;
};

//-------------------------------------------------------------------------------------------------

enum CellType
{
    SIMPLE,
    OBSTACLE,
    GATE,
    OFFICE,
    ENTER
};

class Cell : DrawableObject
{
protected:

    size_t x_ = 0;
    size_t y_ = 0;
    bool passable_ = true;
    Passenger* psg_;
    bool occupied_by_queue_ = false;

public:

    Cell() = default;

    Cell(size_t x, size_t y);

    Cell(size_t x, size_t y, bool passable);

    size_t get_x () const;
    size_t get_y () const;
    bool is_passable () const;

    bool operator==(const Cell& other) const;

    void draw () const override
    {
        if (passable_)
            std::cout << ".";
        else
            std::cout << "#";
    }


    //info about cur passenger
    size_t get_next_psg_step();
    StatusTypes get_psg_status();
    size_t get_psg_aim_ind() {return psg_->get_aim();}

    bool is_occupied()
    {
        if (!psg_)
            return false;
        return true;
    }

    void set_cell_free()
    {
        psg_ = nullptr;
    }
};

//-----------------------------------------------------------

class Gate : public Cell
{
    size_t gatenum_ = 0;

public:

    Gate(size_t x, size_t y, size_t gatenum);

    size_t get_gatenum () const;

    void draw () const override
    {
        std::cout << "{}";
    }
};

//-----------------------------------------------------------

class Enterance : public Cell
{
    size_t enternum_ = 0;

public:

    Enterance(size_t x, size_t y, size_t enternum);

    size_t get_enternum () const;

    void draw () const override
    {
        std::cout << "_";
    }
};

//-----------------------------------------------------------

class RegOffice : public Cell
{
    size_t queue_len_ = 0;

public:

    RegOffice(size_t x, size_t y);

    void take_turn();
    void free_queue_space();

    size_t get_queue_len () const;

    bool operator<(const RegOffice& other) const;

    void draw () const override
    {
        std::cout << "$";
    }
};

//--------------------------------------------------------------------------------------

class Field : DrawableObject
{
    size_t len_ = 0;
    size_t wid_ = 0;
    std::vector<std::unique_ptr<Cell>> cells_;  

    std::vector<size_t> reg_offices_;
    std::vector<size_t> enterances_;
    std::vector<size_t> gates_;
    std::vector<size_t> obstacles_;

public:

    size_t get_len () const;
    size_t get_wid () const;
    const std::vector<size_t>& get_reg_offices () const;
    const std::vector<size_t>& get_enterances () const;
    const std::vector<size_t>& get_gates () const;
    const std::vector<size_t>& get_obstacles () const;

    Field(size_t len, size_t wid, 
          const std::vector<std::pair<size_t, size_t>>& obstacles_pos,
          const std::vector<std::pair<size_t, size_t>>& office_pos,
          const std::vector<std::pair<size_t, size_t>>& entrance_pos,
          const std::vector<std::pair<size_t, size_t>>& gate_pos);

    size_t find_free_reg_office_ind();

    Cell* get_cell_by_ind(size_t ind) const ;

    void draw () const override;

    bool is_passable(size_t ind);
};