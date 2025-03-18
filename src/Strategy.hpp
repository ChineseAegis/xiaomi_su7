#pragma once
#include <cmath> // 引入标准库中的 ceil 函数
using namespace std;
#include <deque>
#include <string>
#include <vector>
#include <algorithm> // 用于max函数
#include <stdexcept>
#include "Controller.hpp"

class Calculate
{
    //计算指定时间片、索引的某个r动作之前有几个r动作。
    static int calculate_num_pre_read_action(const vector<string> &actions, int time, int index);

    //计算连续几步read所消耗的token
    static int computeValue(int base, double factor, int times);

    // 根据行为计算tokens的方式
    static int calculate_tokens(const string &actions, int G, const vector<string> &all_actions, int time);

public:
    // 更新对应时间片的token消耗
    // 输入的 actions 是时间片序列，每个时间片包含一个动作序列
    // tokens 是更新之前记录的每个时间片消耗的 token 数量
    // time 是指定时间片，若未指定，则更新所有时间片的 token
    // index如果指定，则代表all_actions[time][index]为新增动作，只需根据这个新增动作进行更新
    // 返回值：返回更新后的 tokens
    static vector<int> recalculate_tokens(const vector<string> &all_actions, vector<int> tokens, int G, int time = -1, int index = -1);

    //已知一个硬盘的磁头位置，将要读取的所有块的索引，修改传入的action_queue，增加动作到相应时间片中。num v是每个硬盘的存储单元数量
    //static void calculate_actions(int head_index,vector<int> read_queue_indexs,Action_queue& action_queue,int current_time,int num_v);

    
};



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
    int pre_num_r = calculate_num_pre_read_action(all_actions, time, 0);

    if (actions[0] == 'j')
    {
        token_count += G;
        return token_count;
    }

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

vector<int> Calculate::recalculate_tokens(const vector<string> &all_actions, vector<int> tokens, int G, int time, int index)
{
    if (time == -1)
    {
        // 更新所有时间片
        for (size_t i = 0; i < all_actions.size(); i++)
        {
            tokens[i] = calculate_tokens(all_actions[i], G, all_actions, i);
        }
    }
    else if (time && !index)
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
            tokens[time] += 1;
        }
        else if (all_actions[time][index] == 'r')
        {
            int pre_num_r = calculate_num_pre_read_action(all_actions, time, index);
            tokens[time] += max(16, computeValue(64, 0.8, pre_num_r));
        }
    }
    return tokens;
}