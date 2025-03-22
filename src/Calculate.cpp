#include "Calculate.h"

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
    if (actions.size() == 0)
    {
        return token_count;
    }

    if (actions[0] == 'j')
    {
        token_count += G;
        return token_count;
    }
    int pre_num_r = 0;

    for (size_t i = 0; i < actions.size(); i++)
    {
        const char &action = actions[i]; // 获取当前时间片的动作序列

        if (action == 'p')
        {
            token_count += 1;
            pre_num_r = 0;
        }
        else if (action == 'r')
        {
            if (i == 0)
            {
                pre_num_r = calculate_num_pre_read_action(all_actions, time, i);
                token_count += max(16, computeValue(64, 0.8, pre_num_r));
                pre_num_r++;
            }
            else
            {
                token_count += max(16, computeValue(64, 0.8, pre_num_r));
                pre_num_r++;
            }
        }
        else
        {
            throw invalid_argument("Invalid action in time slice: " + action);
        }
    }
    return token_count;
}

vector<int> Calculate::recalculate_tokens(const vector<string> &all_actions, vector<int> tokens, int G, int time, int index, int num)
{
    if (time == -1)
    {
        // 更新所有时间片
        for (size_t i = 0; i < all_actions.size(); i++)
        {
            tokens[i] = calculate_tokens(all_actions[i], G, all_actions, i);
        }
    }
    else if (time>=0 && index==-1)
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
    return tokens;
}

vector<vector<int>> Calculate::calculate_blocks_queue(unordered_map<int, Object> &object_unread_ids, vector<Disk> &disks, int time, int num_v, int G, int num_T)
{
    vector<vector<int>> disk_unread_indexs;
    disk_unread_indexs.resize(disks.size());
    vector<vector<int>> unread_indexs;
    unread_indexs.resize(disks.size());
    for (auto &object_pair : object_unread_ids)
    {
        const Object &object = object_pair.second;
        for (int i = 0; i < REP_NUM; i++)
        {
            for (auto &block : object.blocks[i])
            {
                unread_indexs[block->disk_id].push_back(block->index);
            }
        }
    }
    for (auto &object_pair : object_unread_ids)
    {
        const Object &object = object_pair.second;
        int block_num = object.size;
        vector<int> record;
        record.resize(block_num, INT_MAX);
        vector<pair<int, int>> disk_ids;
        disk_ids.resize(block_num);
        for (int i = 0; i < REP_NUM; i++)
        {
            for (auto &block : object.blocks[i])
            {
                int cost = cost_between_two_index(disks[block->disk_id].head, block->index, unread_indexs[block->disk_id], time, num_v, G, num_T);
                if (cost < record[block->id])
                {
                    record[block->id] = cost;
                    disk_ids[block->id].first = block->disk_id;
                    disk_ids[block->id].second = block->index;
                }
            }
        }
        for (int j = 0; j < block_num; j++)
        {
            disk_unread_indexs[disk_ids[j].first].push_back(disk_ids[j].second);
        }
    }

    return disk_unread_indexs;
}

void Calculate::calculate_actions(int head_index, vector<int> read_queue_indexs, Action_queue &action_queue, int current_time, int num_v, int G)
{

    read_queue_indexs = sort_unread_indexs(head_index, read_queue_indexs, num_v);
    int n = read_queue_indexs.size();
    action_queue.set_current_time(current_time);
    for (size_t i = 0; i < n; i++)
    {
        int distance = Calculate::distance_between_two_index(head_index, read_queue_indexs[i], num_v);
        int action_tokens = distance + action_queue.get_action_tokens(current_time);
        if (action_tokens > G)
        {
            if (action_queue.get_action_tokens(current_time) == 0)
            {
                action_queue.add_jump_action(distance);
                action_queue.current_time_plus_one();
            }
            else
            {
                int rest_tokens = action_tokens - G;

                if (rest_tokens < G)
                {
                    int move = G - action_queue.get_action_tokens(current_time);
                    action_queue.add_pass_action(move);
                    action_queue.current_time_plus_one();
                    action_queue.add_pass_action(rest_tokens);
                }
                else
                {
                    action_queue.current_time_plus_one();
                    action_queue.add_jump_action(distance);
                    action_queue.current_time_plus_one();
                }
            }
        }
        else
        {
            int result=action_queue.add_pass_action(distance);
            
        }
        int decision = action_queue.add_read_action(1); // 用于判断是否读取超过大小
        if (decision != 0 && decision != -1)
        { // 超过大小
            action_queue.current_time_plus_one();
            action_queue.add_read_action(1);
        }
        if(i+1<n)
        {
        head_index = read_queue_indexs[i]+1;
        }
        
    }
}

int Calculate::cost_between_two_index(int head_index, int target_index, vector<int> read_queue_indexs, int current_time, int num_v, int G, int max_T)
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

vector<int> Calculate::sort_unread_indexs(int head, vector<int> read_queue_indexs, int num_v)
{
    sort(read_queue_indexs.begin(), read_queue_indexs.end(), [head, &read_queue_indexs, num_v](int a, int b)
         { return distance_between_two_index(head, a, num_v) < distance_between_two_index(head, b, num_v); });
    return read_queue_indexs;
}
