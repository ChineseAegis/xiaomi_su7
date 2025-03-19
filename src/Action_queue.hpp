#pragma once
#include <iostream>
#include <vector>
#include "Object.hpp"
#include <string>
#include "Strategy.hpp"

using namespace std;

// 表示一个硬盘所有时间片的动作序列
class Action_queue
{
    vector<string> _actions; // 所有时间片的动作
    vector<int> _tokens;     // 已经消耗的token
    int G;
    int end_index = 0;
    void check_end_index(int time);
    void check_back_read_tokens(int time);

public:
    Action_queue(int num_T, int G)
    {
        _actions.resize(num_T);
        _tokens.resize(num_T, 0);
    }

    // 返回所有时间片的动作序列，注意是引用
    vector<string> &get_actions()
    {
        return _actions;
    }

    // 返回指定时间片中，硬盘已经规划了的动作消耗的token数量
    int get_action_tokens(int time);

    // 向该磁盘指定时间片加入pass动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_pass_action(int time, int index = -1);

    // 向该磁盘指定时间片加入read动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_read_action(int time, int index = -1);

    bool add_jump_action(int time, int distance);

    // 删除某个时间片的动作，默认删除该时间片最后一个动作，若指定index，则删除index处的动作
    bool delete_action(int time, int index = -1);

    // 删除某个时间片的所有动作
    bool delete_all_action(int time);
};

void Action_queue::check_end_index(int time)
{
    if (time = end_index - 1 && _actions[time].size() == 0)
    {
        end_index--;
        for (int i = time - 1; i >= 0; i--)
        {
            if (_actions[i].size() > 0)
            {
                break;
            }
            end_index--;
        }
    }
}

void Action_queue::check_back_read_tokens(int time)
{
    int count = time + 1;
    while (count < end_index && ((_actions[count].size() != 0 && _actions[count][0] == 'r') || _actions[count].size() == 0))
    {
        if (_actions[count].size() == 0)
        {
            count++;
            continue;
        }
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, count);
        if (!all_of(_actions[count].begin(), _actions[count].end(), [](char c)
                    { return c == 'r'; }))
        {
            break;
        }
        count++;
    }
}

int Action_queue::get_action_tokens(int time)
{
    if (time >= _actions.size())
    {
        return -1;
    }
    return _tokens[time];
}

bool Action_queue::add_pass_action(int time, int index)
{

    if (time >= _actions.size())
    {
        return -1;
    }
    if (time > end_index)
    {
        end_index = time + 1;
    }
    if (index)
    {
        _actions[time].insert(index, 1, 'p');
        _tokens[time] += 1;
        return true;
    }
    else
    {
        _actions[time].append("p");
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, time, G);
        return true;
    }
}

bool Action_queue::add_read_action(int time, int index)
{
    if (time >= _actions.size())
    {
        return -1;
    }
    if (time > end_index)
    {
        end_index = time + 1;
    }
    if (index)
    {
        _actions[time].insert(index, 1, 'r');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, time, G, index);
        return true;
    }
    else
    {
        _actions[time].append("r");
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, time, G);
        if (time + 1 < end_index)
        {
            check_back_read_tokens(time);
        }
        return true;
    }
}

bool Action_queue::add_jump_action(int time, int distance)
{
    if (time >= _actions.size())
    {
        return -1;
    }
    if (_actions[time].size() > 0)
    {
        return -1;
    }
    else
    {
        _actions[time] = "j " + to_string(distance);
    }
    if (time > end_index)
    {
        end_index = time + 1;
    }
    _tokens[time] = G;
}

bool Action_queue::delete_action(int time, int index = -1)
{
    if (time < 0 || time >= _actions.size() || _actions[time].size() == 0)
    {
        return false;
    }
    if (index)
    {
        if (_actions[time][0] == 'j' || index < 0 || index >= _actions[time].size())
        {
            return false;
        }
        char s = _actions[time][index];
        auto begin = _actions[time].begin();
        _actions[time].erase(begin + index);
        if (s == 'p')
        {
            _tokens[time]--;
        }
        else
        {
            _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, time);
        }

        check_end_index(time);
    }
    else
    {
        if (_actions[time][0] == 'j')
        {
            _actions[time] = "";
            _tokens[time] = 0;
            check_back_read_tokens(time);
            check_end_index(time);
            return true;
        }
        char s = _actions[time][_actions.size() - 1];
        _actions[time].pop_back();
        if (s == 'p')
        {
            _tokens[time]--;
        }
        else
        {
            _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, time);
            check_back_read_tokens(time);
        }
        check_end_index(time);
    }

    return true;
}

bool Action_queue::delete_all_action(int time)
{
    if (time < 0 || time >= _actions.size() || _actions[time].size() == 0)
    {
        return false;
    }
    _actions[time] = "";
    _tokens[time] = 0;
    check_end_index(time);
    return true;
}