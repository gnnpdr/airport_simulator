#pragma once
#include "common.hpp"
#include "astar.hpp"
#include "psg.hpp"
#include <vector>
#include <memory>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <queue>
#include "visitor.hpp"

class Passenger;

//-------------------------------------------------------------------------------------------------

//нужен для того, чтобы довольно просто устроить конструктор поля
enum CellType
{
    SIMPLE,
    OBSTACLE,
    GATE,
    OFFICE,
    ENTER
};

class Cell : GameObject
{
protected:

    size_t x_ = 0;
    size_t y_ = 0;
    bool passable_ = true;

    std::weak_ptr<Passenger> psg_;

public:

    Cell() = default;
    Cell(size_t x, size_t y);
    Cell(size_t x, size_t y, bool passable);

    void set_x(size_t x) {x_ = x;}
    void set_y(size_t y) {y_ = y;}
    size_t get_x() {return x_;}
    size_t get_y() {return y_;}
    bool is_passable () const;

    bool operator==(const Cell& other) const;

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }

    bool is_occupied() const;
    void set_cell_free();
    void set_psg(std::shared_ptr<Passenger> psg);

    size_t get_next_psg_step();
    StatusTypes get_psg_status();
    size_t get_psg_aim_ind();

    std::shared_ptr<Passenger> get_psg() const;
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

/*class RegOffice : public Cell
{
    std::deque<std::shared_ptr<Passenger>> queue_;

public:

    RegOffice(size_t x, size_t y);

    void take_turn();
    void free_queue_space();

    size_t get_queue_len () const;

   
};*/

class RegOffice : public Cell 
{
private:

    int service_time_ = 0;
    const int SERVICE_DURATION = 3;
    std::queue<std::weak_ptr<Passenger>> queue_;
    
public:

    RegOffice(size_t x, size_t y) : Cell(x, y, true) {} 

    int get_service_time ()
    {
        return service_time_;
    }

    void update() 
    {
        service_time_--;
        if (service_time_ <= 0) 
        {
            std::cout << "RegOffice is free now" << std::endl;
        }
    }

    //bool is_occupied() const 
    //{
    //    return Cell::is_occupied() || service_time_ > 0;
    //}

    bool is_occupied() const 
    {
        bool cell_occupied = Cell::is_occupied();
        bool service_busy = service_time_ > 0;

        std::cout << "RegOffice is_occupied: cell=" << cell_occupied 
                  << ", service=" << service_busy 
                  << ", result=" << (cell_occupied || service_busy) << std::endl;

        if (cell_occupied) {
            auto psg = get_psg();
            if (psg) {
                std::cout << "  Passenger " << psg->get_num() 
                          << " status " << psg->get_status() << std::endl;
            }
        }

        return cell_occupied || service_busy;
    }
    
    void take_turn(std::shared_ptr<Passenger> psg);
    
    
    void add_to_queue(std::shared_ptr<Passenger> psg) 
    {
        queue_.push(psg);
        //std::cout << "Passenger added to queue. Queue size: " << queue_.size() << std::endl;
    }
    
    bool is_next_in_queue(std::shared_ptr<Passenger> psg) 
    {
        if (queue_.empty()) 
            return false;

        if (auto next = queue_.front().lock()) 
            return next == psg;
        return false;
    }
    
    void pop_queue() 
    {
        if (!queue_.empty())
            queue_.pop();
    }
    
    void free_queue_space()  
    {
        pop_queue();
    }

    bool operator<(const RegOffice& other) const;

    size_t get_queue_len () const {return queue_.size();}

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

    std::vector<std::shared_ptr<Passenger>> passengers_;
    std::deque<std::shared_ptr<Passenger>> entrance_queue_;
    std::vector<size_t> free_enterances_;

    bool is_paused_ = false;
    float simulation_time_ = 0.0f;

public:

    size_t get_heig () const;
    size_t get_wid () const;
    Cell* get_cell_by_ind(size_t ind) const ;
    std::vector<size_t>& get_obstacles();
    std::vector<size_t>& get_reg_offices()
    {
        return reg_offices_;
    }

    Field(size_t heig, size_t wid, 
          const std::vector<std::pair<size_t, size_t>>& obstacles_pos,
          const std::vector<std::pair<size_t, size_t>>& office_pos,
          const std::vector<std::pair<size_t, size_t>>& entrance_pos,
          const std::vector<std::pair<size_t, size_t>>& gate_pos);

    size_t find_free_reg_office_ind();

    std::shared_ptr<Passenger> add_passenger(size_t num);

    void accept(Visitor& visitor) override 
    {
        visitor.visit(this);
    }

    void togglePause() 
    {
        is_paused_ = !is_paused_;
    }

    void update(float delta_time);
    void set_paths();
    void reset_statuses();
    void solve_collisions();
    void plan_steps();
    void step();


    size_t reg();
    PathSituation check_next_step(size_t psg_aim, size_t psg_cur_ind, size_t next_step_ind);
    std::vector<size_t> update_obstacles_by_opponent(size_t opponents_coord);
    std::vector<size_t> update_obstacles_by_queues(size_t aim_reg_office_ind);

    //void end_of_path(std::shared_ptr<Passenger> psg);
    void remove_psg(std::shared_ptr<Passenger> psg);
    void move_psg(std::shared_ptr<Passenger> psg);
    void set_to_line(std::shared_ptr<Passenger> psg);
    void get_around_queue(std::shared_ptr<Passenger> psg);
    void process_entrance_queue();
};