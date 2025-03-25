#include "Calculate.h"
#include <chrono>
#include <fstream>

int Calculate::calculate_num_pre_read_action(const vector<string> &actions, int time, int index)
{
    int count = 0;
    for (int j = time; j >= 0; j--)
    {
        for (int i = index - 1; i >= 0; i--)
        {
            if (actions[j][i] == 'r')
            {
                count++;
            }
            else
            {
                return count;
            }
        }
        if (j - 1 >= 0)
        {
            index = actions[j - 1].size();
        }
    }
    return count;
}

int Calculate::computeValue(int base, double factor, int times)
{
    double result = base;
    for (int i = 0; i < times; i++)
    {
        result = std::ceil(result * factor); // 每次先乘 factor，再取 ceil
    }
    return static_cast<int>(result); // 转换为整数
}

int Calculate::calculate_tokens(const string &actions, int G, const vector<string> &all_actions, int time)
{
    int token_count = 0;
    if (actions.empty())
        return 0;
    if (actions[0] == 'j')
        return G;

    int pre_time_r = calculate_num_pre_read_action(all_actions, time, 0);
    int current_r_chain = 0;

    for (size_t i = 0; i < actions.size(); ++i)
    {
        const char &action = actions[i];
        if (action == 'p')
        {
            token_count += 1;
            pre_time_r = current_r_chain = 0;
        }
        else if (action == 'r')
        {
            current_r_chain = (i > 0 && actions[i - 1] == 'r') ? current_r_chain + 1 : 1;
            int total_pre_r = pre_time_r + current_r_chain - 1;
            token_count += max(16, computeValue(64, 0.8, total_pre_r));
        }
        else
        {
            throw invalid_argument("Invalid action: " + string(1, action));
        }
    }
    return token_count;
}

void Calculate::recalculate_tokens(const vector<string> &all_actions, vector<int> &tokens, int G, int time, int index, int num)
{
    if (time == -1)
    {
        // 更新所有时间片
        for (size_t i = 0; i < all_actions.size(); i++)
        {
            tokens[i] = calculate_tokens(all_actions[i], G, all_actions, i);
        }
    }
    else if (time >= 0 && index == -1)
    {
        // 更新指定时间片
        if (time >= 0 && time < static_cast<int>(all_actions.size()))
        {
            tokens[time] = calculate_tokens(all_actions[time], G, all_actions, time);
        }
    }
    else
    {
        if (all_actions[time][index] == 'p')
        {
            if ((index - 1 >= 0 && index + num < all_actions[time].size() && all_actions[time][index - 1] == 'r' && all_actions[time][index + num] == 'r') || (index == 0 && index + num < all_actions[time].size() && all_actions[time][index + num] == 'r' && time - 1 >= 0 && all_actions[time - 1].size() > 0 && all_actions[time - 1][all_actions[time - 1].size() - 1] == 'r'))
            {
                tokens[time] = calculate_tokens(all_actions[time], G, all_actions, time);
            }
            else
            {
                tokens[time] += num;
            }
        }
        else if (all_actions[time][index] == 'r')
        {
            // int pre_num_r = calculate_num_pre_read_action(all_actions, time, index+num);
            // tokens[time] += max(16, computeValue(64, 0.8, pre_num_r));

            tokens[time] = calculate_tokens(all_actions[time], G, all_actions, time);
        }
    }
}

struct pair_hash
{
    std::size_t operator()(const std::pair<int, int> &p) const
    {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
    }
};

void Calculate::calculate_blocks_queue(
    const std::list<ReadRequest> &read_request_list,
    const std::unordered_map<int, Object> &objects,
    std::vector<Disk> &disks,
    std::vector<std::deque<int>> &disk_unread_indexs,
    int time,
    int num_v,
    int G,
    int num_T)
{
    std::vector<std::deque<int>> temp(disks.size());
    disk_unread_indexs.swap(temp);

    std::unordered_set<std::pair<int, int>, pair_hash> blocks;

    for (const auto &req : read_request_list)
    {
        int object_id = req.object_id;
        const Object &object = objects.at(object_id);
        int block_num = object.size;

        std::vector<int> record(block_num, INT_MAX);
        std::vector<std::pair<int, int>> disk_ids(block_num); // <disk_id, index>

        for (int i = 0; i < REP_NUM; ++i)
        {
            for (auto *block : object.blocks[i])
            {
                int block_id = block->id;
                if (req.blocks[block_id] == 1)
                    continue;

                int cost = distance_between_two_index(disks[block->disk_id].head, block->index, num_v);
                if (cost < record[block_id])
                {
                    record[block_id] = cost;
                    disk_ids[block_id] = {block->disk_id, block->index};
                }
            }
        }

        for (int j = 0; j < block_num; ++j)
        {
            int disk_id = disk_ids[j].first;
            int index = disk_ids[j].second;

            if (blocks.insert({disk_id, index}).second)
            {
                disk_unread_indexs[disk_id].push_back(index);
            }
        }
    }
}

void Calculate::append_blocks_for_new_requests(
    const std::vector<int> &new_request_ids,
    const std::unordered_map<int, std::list<ReadRequest>::iterator> &object_request_iters,
    const std::unordered_map<int, Object> &objects,
    std::vector<Disk> &disks,
    std::vector<std::deque<int>> &disk_unread_indexs,
    const unordered_map<int, vector<int>> &object_unread_ids,
    int num_v)
{
    std::unordered_set<std::pair<int, int>, pair_hash> added_blocks;

    // 填充已存在的 (disk_id, index)
    for (int disk_id = 0; disk_id < disks.size(); ++disk_id) {
        for (int index : disk_unread_indexs[disk_id]) {
            added_blocks.insert({disk_id, index});
        }
    }

    for (int request_id : new_request_ids) {
        auto it = object_request_iters.find(request_id);
        if (it == object_request_iters.end()) continue;

        const ReadRequest &req = *(it->second);
        const Object &object = objects.at(req.object_id);

        int block_num = object.size;
        std::vector<int> record(block_num, INT_MAX);
        std::vector<std::pair<int, int>> disk_ids(block_num);

        for (int i = 0; i < REP_NUM; ++i) {
            for (auto *block : object.blocks[i]) {
                int block_id = block->id;
                if (req.blocks[block_id] == 1) continue;

                int cost = distance_between_two_index(disks[block->disk_id].head, block->index, num_v);
                if (cost < record[block_id]) {
                    record[block_id] = cost;
                    disk_ids[block_id] = {block->disk_id, block->index};
                }
            }
        }

        for (int j = 0; j < block_num; ++j) {
            int disk_id = disk_ids[j].first;
            int index = disk_ids[j].second;
            if (added_blocks.find({disk_id, index}) == added_blocks.end()) {
                added_blocks.insert({disk_id, index});
                disk_unread_indexs[disk_id].push_back(index);
            }
        }
    }
}

int Calculate::calculate_actions(int head_index, deque<int> &read_queue_indexs, Action_queue &action_queue, int current_time, int num_v, int G, bool is_continue)
{

    // read_queue_indexs = sort_unread_indexs(head_index, read_queue_indexs, num_v);
    int n = (read_queue_indexs.size() < 5) ? read_queue_indexs.size() : 5;
    action_queue.set_current_time(current_time, is_continue);
    for (size_t i = 0; i < n; i++)
    {
        int distance = Calculate::distance_between_two_index(head_index, read_queue_indexs[i], num_v);
        int action_tokens = distance + action_queue.get_action_tokens(current_time);
        if (action_tokens > G)
        {
            if (action_queue.get_action_tokens(current_time) == 0)
            {
                action_queue.add_jump_action(read_queue_indexs[i]);
                action_queue.current_time_plus_one();
                current_time++;
            }
            else
            {
                int rest_tokens = action_tokens - G;

                if (rest_tokens < G)
                {
                    int move = G - action_queue.get_action_tokens(current_time);
                    action_queue.add_pass_action(move);
                    action_queue.current_time_plus_one();
                    current_time++;
                    action_queue.add_pass_action(rest_tokens);
                }
                else
                {
                    action_queue.current_time_plus_one();
                    current_time++;
                    action_queue.add_jump_action(read_queue_indexs[i]);
                    current_time++;
                    action_queue.current_time_plus_one();
                }
            }
        }
        else
        {
            int result = action_queue.add_pass_action(distance);
        }
        int decision = action_queue.add_read_action(1); // 用于判断是否读取超过大小
        if (decision != 0 && decision != -1)
        { // 超过大小
            action_queue.current_time_plus_one();
            current_time++;
            action_queue.add_read_action(1);
        }
        head_index = read_queue_indexs[i] + 1;
    }
    for (int i = 0; i < n; i++)
    {
        read_queue_indexs.pop_front();
    }
    return head_index;
}

int Calculate::cost_between_two_index(int head_index, int target_index, deque<int> read_queue_indexs, int current_time, int num_v, int G, int max_T)
{
    read_queue_indexs.push_back(target_index);
    Action_queue actions(max_T, G);
    calculate_actions(head_index, read_queue_indexs, actions, 0, num_v, G);
    return actions.get_current_time();
}

int Calculate::distance_between_two_index(int begin_index, int end_index, int num_v)
{
    // 边界检查：防止非法索引
    if (begin_index < 0 || begin_index >= num_v ||
        end_index < 0 || end_index >= num_v || num_v <= 0)
    {
        return -1;
    }

    // 使用模运算计算循环距离（C-SCAN方式）
    return (end_index - begin_index + num_v) % num_v;
}

deque<int> Calculate::sort_unread_indexs(int head, const deque<int> &indexes, int num_v, int n)
{
    vector<pair<int, int>> indexed_dist;
    indexed_dist.reserve(indexes.size());

    for (int idx : indexes)
        indexed_dist.emplace_back(idx, distance_between_two_index(head, idx, num_v));

    std::partial_sort(indexed_dist.begin(), indexed_dist.begin() + std::min(n, (int)indexed_dist.size()), indexed_dist.end(),
                      [](const auto &a, const auto &b)
                      {
                          return a.second < b.second;
                      });

    deque<int> sorted;
    for (int i = 0; i < n && i < indexed_dist.size(); ++i)
        sorted.push_back(indexed_dist[i].first);

    // 保留剩下的元素
    for (int i = n; i < indexed_dist.size(); ++i)
        sorted.push_back(indexed_dist[i].first);

    return sorted;
}
