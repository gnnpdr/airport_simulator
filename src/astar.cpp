#include "astar.hpp"

AStarPathFinder::AStarPathFinder(Field& field) : field_(field), obstacles_(field_.get_obstacles()) {}
AStarPathFinder::AStarPathFinder(Field& field, std::vector<size_t> obstacles) : field_(field), obstacles_(obstacles) {}

bool AStarCell::operator>(const AStarCell& other) const 
{
    return perf_eval_ > other.perf_eval_;
}

bool AStarCell::operator==(const AStarCell& other) const
{
    return ind_ == other.ind_;
}

bool AStarCell::operator<(const AStarCell& other) const
{
    if (perf_eval_ != other.perf_eval_)
        return perf_eval_ < other.perf_eval_;
    return ind_ < other.ind_;
}

std::vector<size_t> AStarPathFinder::find_path(size_t start, size_t aim)
{
    size_t aim_dist = find_aim_dist(start, aim);
    open_cells_.emplace(start, 0, aim_dist, 0 + aim_dist);
    parents_[start] = start;
    while (!open_cells_.empty())
    {
        //std::cout << "aim " << aim << std::endl;
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
            
        update_open_cells(cur, aim);
    }
    return {};
}

std::vector<size_t> AStarPathFinder::reconstruct_path (size_t start, size_t aim)
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



void AStarPathFinder::update_open_cells(AStarCell& cur, size_t aim)
{
    std::pair<int, int> cur_coord = f1dto2d(cur.ind_, field_.get_wid());
    for (auto& step : steps)
    {
        std::pair<int, int> new_coord = {cur_coord.first + step.first, cur_coord.second + step.second};
        //std::cout << "new step " << new_coord.first << ", " << new_coord.second << std::endl;
        
        if (new_coord.first < 0 || new_coord.second < 0 || new_coord.first >= static_cast<int>(field_.get_wid()) || new_coord.second >= static_cast<int>(field_.get_heig()))
        {
            continue;
        }
        size_t new_start = f2dto1d(new_coord.first, new_coord.second, field_.get_wid());
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
        size_t aim_dist = find_aim_dist(new_start, aim);
        //std::cout << "aim_dist " << aim_dist << std::endl;
        open_cells_.emplace(new_start, start_dist, aim_dist, start_dist + aim_dist);
        parents_[new_start] = cur.ind_;
    }
}

size_t AStarPathFinder::find_aim_dist(size_t start, size_t aim)
{
    std::pair<size_t, size_t> start_coords = f1dto2d(start, field_.get_wid());
    std::pair<size_t, size_t> aim_coords = f1dto2d(aim, field_.get_wid());
    size_t x_dist = abs(start_coords.first - aim_coords.first);
    size_t y_dist = abs(start_coords.second - aim_coords.second);
    return (x_dist + y_dist) * 10;
}

void AStarPathFinder::print_path()
{
    for (size_t step : path_)
    {
        std::pair<size_t, size_t> step_coords = f1dto2d(step, field_.get_wid());
        std::cout << step_coords.first << ", " << step_coords.second << " / ";
    }
    std::cout << std::endl;
}