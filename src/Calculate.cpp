#include "Calculate.h"
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
    disk_unread_indexs.assign(disks.size(), {});
    disk_unread_indexs.reserve(num_v);
    std::mutex mtx;

    const int num_threads = 8;
    std::vector<const ReadRequest *> request_ptrs;
    request_ptrs.reserve(read_request_list.size());

    // 避免拷贝，存储指针
    for (const auto &req : read_request_list)
        request_ptrs.push_back(&req);

    size_t total = request_ptrs.size();
    size_t chunk_size = (total + num_threads - 1) / num_threads;

    std::vector<std::thread> threads;

    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back([&, t]()
                             {
            size_t start = t * chunk_size;
            size_t end = std::min(start + chunk_size, total);

            // 使用 vector<deque>，后续可以 move 或 splice 到主线程
            std::vector<std::deque<int>> local_disk_queues(disks.size());

            for (size_t i = start; i < end; ++i)
            {
                const ReadRequest &req = *request_ptrs[i];
                int object_id = req.object_id;
                const Object &object = objects.at(object_id);
                int block_num = object.size;

                std::vector<int> record(block_num, INT_MAX);
                std::vector<std::pair<int, int>> disk_ids(block_num);

                for (int j = 0; j < REP_NUM; ++j)
                {
                    for (auto *block : object.blocks[j])
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
                    local_disk_queues[disk_id].push_back(index);
                }
            }

            // 合并线程数据，避免 copy，用 splice 或 move
            std::lock_guard<std::mutex> lock(mtx);
            for (size_t d = 0; d < disks.size(); ++d)
            {
                disk_unread_indexs[d].insert(
                    disk_unread_indexs[d].end(),
                    std::make_move_iterator(local_disk_queues[d].begin()),
                    std::make_move_iterator(local_disk_queues[d].end())
                );
            } });
    }

    for (auto &th : threads)
        th.join();
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
    std::unordered_set<std::pair<int, int>, pair_hash> blocks;

    for (int i = 0; i < disks.size(); i++)
    {
        for (auto &index : disk_unread_indexs[i])
        {
            int object_id = disks[i].units[index]->object_id;
            int block_id = disks[i].units[index]->id;
            added_blocks.insert({object_id, block_id});
        }
    }
    // for(int i=0;i<disks.size();i++)
    // {
    //     for(auto& index:disk_unread_indexs[i])
    //     {
    //         blocks.insert({i,index});
    //     }
    // }

    for (int request_id : new_request_ids)
    {
        auto it = object_request_iters.find(request_id);
        if (it == object_request_iters.end())
            continue;

        const ReadRequest &req = *(it->second);
        const Object &object = objects.at(req.object_id);

        const vector<int> &request_ids = object_unread_ids.at(object.id);

        int block_num = object.size;

        std::vector<int> record(block_num, INT_MAX);
        std::vector<std::pair<int, int>> disk_ids(block_num);

        for (int i = 0; i < REP_NUM; ++i)
        {
            for (auto *block : object.blocks[i])
            {
                // bool skip = false;
                // for (auto &r_id : request_ids)
                // {
                //     if (r_id != request_id &&
                //         object_request_iters.at(r_id)->blocks[block->id] == 0)
                //     {
                //         skip = true;
                //         break;
                //     }
                // }
                // if (skip)
                //     continue;

                int block_id = block->id;
                // if (req.blocks[block_id] == 1) continue;

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
            int object_id = disks[disk_id].units[index]->object_id;
            int block_id = disks[disk_id].units[index]->id;
            if (added_blocks.insert({object_id, block_id}).second)
            {
                disk_unread_indexs[disk_id].push_back(index);
            }
        }
    }
}

int Calculate::calculate_actions(int head_index, deque<int> &read_queue_indexs, Action_queue &action_queue, int current_time, int num_v, int G, bool is_continue)
{

    // read_queue_indexs = sort_unread_indexs(head_index, read_queue_indexs, num_v);
    int n = (read_queue_indexs.size() < 1) ? read_queue_indexs.size() : 1;
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

inline int Calculate::cost_between_two_index(int head_index, int target_index, deque<int> read_queue_indexs, int current_time, int num_v, int G, int max_T)
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
        throw runtime_error(to_string(begin_index)+" "+to_string(end_index)+" "+to_string(num_v));
        return -1;
    }

    // 使用模运算计算循环距离（C-SCAN方式）
    return (end_index - begin_index + num_v) % num_v;
}

deque<int> Calculate::sort_unread_indexs(int head, const deque<int> &indexes, int num_v, int n)
{
    const int MAX_DIST = num_v;
    vector<vector<int>> buckets(MAX_DIST + 1);
    vector<bool> seen(num_v, false);

    // int unique_size = 0;
    for (int idx : indexes)
    {
        if (!seen[idx])
        {
            seen[idx] = true;
            //++unique_size;

            int dist = distance_between_two_index(head, idx, num_v);
            buckets[dist].push_back(idx);
        }
    }

    // n = std::min(n, unique_size);

    deque<int> sorted;
    for (int d = 0; d <= MAX_DIST; ++d)
    {
        for (int idx : buckets[d])
        {
            sorted.push_back(idx);
        }
    }

    return sorted;
}
