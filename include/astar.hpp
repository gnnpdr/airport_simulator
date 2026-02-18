#pragma once
#include "common.hpp"
#include "field.hpp"
#include <unordered_set>
#include <vector>
#include <functional>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>

class Field;

struct AStarCell
{
    size_t ind_;
    size_t start_dis_;
    size_t aim_dist_;
    size_t perf_eval_; 

    bool operator>(const AStarCell& other) const;
    bool operator==(const AStarCell& other) const;
    bool operator<(const AStarCell& other) const;
};

class AStarPathFinder
{
    std::priority_queue<AStarCell, std::vector<AStarCell>, std::greater<AStarCell>> open_cells_;
    std::unordered_set<size_t> closed_cells_;
    std::unordered_map<size_t, size_t> parents_;

    Field& field_;
    size_t aim_;
    size_t start_;
    std::vector<size_t> path_;
    std::vector<size_t> obstacles_;

public:

    AStarPathFinder(size_t aim, size_t start, Field& field);
    AStarPathFinder(size_t aim, size_t start, Field& field, std::vector<size_t> obstacles);

    std::vector<size_t> find_path();
    void print_path();

private:

    std::vector<size_t> reconstruct_path (size_t start, size_t aim);
    void update_open_cells(AStarCell& cur, size_t aim);
    size_t find_aim_dist(size_t start, size_t aim);
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