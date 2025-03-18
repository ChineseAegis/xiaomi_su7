#pragma once
#include <iostream>
#include <vector>
#include "Object.hpp"
#include <string>
#include"Strategy.hpp"

using namespace std;


// 表示一个硬盘所有时间片的动作序列
class Action_queue
{
    vector<string> _actions; //
    vector<int> _tokens;     // 已经消耗的token
    int G;

public:
    Action_queue(int num_T, int G)
    {
        _actions.resize(num_T);
        _tokens.resize(num_T, 0);
    }

    //返回所有时间片的动作序列，注意是引用
    vector<string>& get_actions()
    {
        return _actions;
    }

    // 返回指定时间片中，硬盘已经规划了的动作所消耗的token数量
    int get_action_tokens(int time);

    // 向该磁盘指定时间片加入pass动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_pass_action(int time, int index = -1);

    // 向该磁盘指定时间片加入read动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_read_action(int time, int index = -1);

    bool add_jump_action(int time, int distance);
};

int Action_queue::get_action_tokens(int time)
{
    if (time >= _actions.size())
    {
        return -1;
    }
    return _tokens[time];
}

bool Action_queue::add_pass_action(int time, int index = -1)
{
    if (time >= _actions.size())
    {
        return -1;
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

bool Action_queue::add_read_action(int time, int index = -1)
{
    if (time >= _actions.size())
    {
        return -1;
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
    _tokens[time] = G;
}
