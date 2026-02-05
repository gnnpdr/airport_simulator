#pragma once
#include "field.hpp"
#include "common.hpp"
#include <unordered_set>
#include <vector>
#include <functional>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>


struct AStarCell
{
    size_t ind_;
    size_t start_dis_;
    size_t aim_dist_;
    size_t perf_eval_; 

    bool operator>(const AStarCell& other) const 
    {
        return perf_eval_ > other.perf_eval_;
    }

    bool operator==(const AStarCell& other) const
    {
        return ind_ == other.ind_;
    }
    
    bool operator<(const AStarCell& other) const
    {
        if (perf_eval_ != other.perf_eval_)
            return perf_eval_ < other.perf_eval_;

        return ind_ < other.ind_;
    }
};

inline void print_priority_queue(std::priority_queue<AStarCell, std::vector<AStarCell>, std::greater<AStarCell>>& original_pq) 
{
    std::priority_queue<AStarCell, std::vector<AStarCell>, std::greater<AStarCell>> pq_copy = original_pq;
    
    std::cout << "open_cells: ";
    while (!pq_copy.empty()) 
    {
        std::cout << pq_copy.top().ind_  << " ";
        pq_copy.pop();
    }
    std::cout << "\n";
}

class AStarPathFinder
{
    std::priority_queue<AStarCell, std::vector<AStarCell>, std::greater<AStarCell>> open_cells_;
    std::unordered_set<size_t> closed_cells_;
    std::unordered_map<size_t, size_t> parents_;

    std::vector<size_t> path_;
    std::vector<size_t> obstacles_;

public:

    AStarPathFinder(Field& field) : obstacles_(field.get_obstacles()) {}
    AStarPathFinder(std::vector<size_t> obstacles) : obstacles_(obstacles) {} 

    std::vector<size_t> find_path(Field& field, size_t start, size_t aim)
    {
        size_t aim_dist = find_aim_dist(field, start, aim);
        open_cells_.emplace(start, 0, aim_dist, 0 + aim_dist);
        parents_[start] = start;

        while (!open_cells_.empty())
        {
            std::cout << "aim " << aim << std::endl;
            //print_priority_queue(open_cells_);

            AStarCell cur = open_cells_.top();
            open_cells_.pop();

            if (closed_cells_.contains(cur.ind_)) 
            {
                std::cout << "closed" << std::endl;
                continue;
            }

            closed_cells_.insert(cur.ind_);

            if (cur.ind_ == aim)
            {
                path_ = reconstruct_path(start, aim);
                return path_;
            }
                
            update_open_cells(cur, field, aim);
        }

        return {};
    }

    std::vector<size_t> reconstruct_path (size_t start, size_t aim)
    {
        std::vector<size_t> path;
        size_t cur = aim;

        while (cur != start)
        {
            path.push_back(cur);
            auto prev = parents_.find(cur);
            if (prev == parents_.end())
                return {};
            cur = prev->second;
        }
        path.push_back(start);
        std::reverse(path.begin(), path.end());

        return path;
    }

    const std::vector<std::pair<int, int>> steps = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1}};

    void update_open_cells(AStarCell& cur, Field& field, size_t aim)
    {
        std::pair<int, int> cur_coord = f1dto2d(cur.ind_, field.get_wid());

        for (auto& step : steps)
        {
            std::pair<int, int> new_coord = {cur_coord.first + step.first, cur_coord.second + step.second};
            //std::cout << "new step " << new_coord.first << ", " << new_coord.second << std::endl;
            
            if (new_coord.first < 0 || new_coord.second < 0)
            {
                //std::cout << "less" << std::endl;
                continue;
            }

            if (new_coord.first > field.get_wid() || new_coord.second > field.get_len())
            {
                //std::cout << "more" << std::endl;
                continue;
            }

            size_t new_start = f2dto1d(new_coord.first, new_coord.second, field.get_wid());

            if (closed_cells_.contains(new_start))
            {
                //std::cout << "closed" << std::endl;
                continue;
            }

            if (std::find(obstacles_.begin(), obstacles_.end(), new_start) != obstacles_.end())
            {
                //std::cout << "not passeble" << std::endl;
                continue;
            }

            size_t offset = (step.first == 0 || step.second == 0) ? 10 : 14;
            //std::cout << "offset " << offset << std::endl;
            size_t start_dist = cur.start_dis_ + offset;
            //std::cout << "start_dist " << start_dist << std::endl;
            size_t aim_dist = find_aim_dist(field, new_start, aim);
            //std::cout << "aim_dist " << aim_dist << std::endl;

            open_cells_.emplace(new_start, start_dist, aim_dist, start_dist + aim_dist);
            parents_[new_start] = cur.ind_;
        }
    }

    size_t find_aim_dist(Field& field, size_t start, size_t aim)
    {
        std::pair<size_t, size_t> start_coords = f1dto2d(start, field.get_wid());
        std::pair<size_t, size_t> aim_coords = f1dto2d(aim, field.get_wid());

        size_t x_dist = abs(start_coords.first - aim_coords.first);
        size_t y_dist = abs(start_coords.second - aim_coords.second);

        return (x_dist + y_dist) * 10;
    }

    void print_path(Field& field)
    {
        for (size_t step : path_)
        {
            std::pair<size_t, size_t> step_coords = f1dto2d(step, field.get_wid());

            std::cout << step_coords.first << ", " << step_coords.second << " / ";
        }

        std::cout << std::endl;
    }
};

