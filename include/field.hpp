#pragma once
#include "common.hpp"
#include "psg.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "visitor.hpp"

class Passenger;


//-------------------------------------------------------------------------------------------------

enum CellType
{
    SIMPLE,
    OBSTACLE,
    GATE,
    OFFICE,
    ENTER
};

const size_t CELL_SIZE = 1;

class Cell : GameObject
{
protected:

    size_t x_ = 0;
    size_t y_ = 0;
    bool passable_ = true;
    Passenger* psg_ = nullptr;
    //bool occupied_by_queue_ = false;

public:

    Cell() = default;
    Cell(size_t x, size_t y);
    Cell(size_t x, size_t y, bool passable);

    size_t get_x () const;
    size_t get_y () const;
    bool is_passable () const;
    bool is_occupied();
    void set_cell_free();

    //info about cur passenger
    size_t get_next_psg_step();
    StatusTypes get_psg_status();
    size_t get_psg_aim_ind();
    bool is_psg_nullptr();

    Passenger* get_psg();

    bool operator==(const Cell& other) const;

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};

//-----------------------------------------------------------

class Gate : public Cell
{
    size_t gatenum_ = 0;

public:

    Gate(size_t x, size_t y, size_t gatenum);
    size_t get_gatenum () const;

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};

//-----------------------------------------------------------

class Enterance : public Cell
{
    size_t enternum_ = 0;

public:

    Enterance(size_t x, size_t y, size_t enternum);
    size_t get_enternum () const;

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
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

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }
};

//--------------------------------------------------------------------------------------

class Field : GameObject
{
    size_t heig_ = 0;
    size_t wid_ = 0;
    std::vector<std::unique_ptr<Cell>> cells_;  

    std::vector<size_t> reg_offices_;
    std::vector<size_t> enterances_;
    std::vector<size_t> gates_;
    std::vector<size_t> obstacles_;

public:

    size_t get_heig () const;
    size_t get_wid () const;
    const std::vector<size_t>& get_reg_offices () const;
    const std::vector<size_t>& get_enterances () const;
    const std::vector<size_t>& get_gates () const;
    const std::vector<size_t>& get_obstacles () const;
    bool is_passable(size_t ind);
    Cell* get_cell_by_ind(size_t ind) const ;

    Field(size_t heig, size_t wid, 
          const std::vector<std::pair<size_t, size_t>>& obstacles_pos,
          const std::vector<std::pair<size_t, size_t>>& office_pos,
          const std::vector<std::pair<size_t, size_t>>& entrance_pos,
          const std::vector<std::pair<size_t, size_t>>& gate_pos);

    size_t find_free_reg_office_ind();

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }

    PathSituation check_next_step(size_t psg_aim, size_t psg_cur_ind, size_t next_step_ind);
    bool is_there_others_interested(size_t next_step_ind);
    std::vector<size_t> update_obstacles_by_opponent(size_t opponents_coord);
    std::vector<size_t> update_obstacles_by_queues();
};