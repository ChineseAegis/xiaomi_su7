#pragma once
#include <iostream>
#include <vector>
#include "Object.hpp"
#include <string>
#include "Strategy.hpp"

using namespace std;

// 表示一个硬盘所有时间片的动作序列。
// 动作只能按照时间片从左往右顺序写入，删除时只能从右往左顺序删除。
// 当前时间片代表了目前你正在操作哪个时间片。
// 当前时间片左边的时间片的动作无法修改，右边的时间片的动作永远是空的。
// 如果当前时间片写入完毕，可以手动调用current_time_plus_one()将时间片向右移动一个单位。
// 调用delete_current_rime_actions()课将当前时间片的动作清空并自动向左移动一个单位。
class Action_queue
{
    private:
    vector<string> _actions; // 所有时间片的动作
    vector<int> _tokens;     // 已经消耗的token
    int G;
    int current_index = 0;
    void check_end_index(int time);
    void check_back_read_tokens(int time);

public:
    Action_queue(int num_T, int G)
    {
        _actions.resize(num_T);
        _tokens.resize(num_T, 0);
    }

    // 返回所有时间片的动作序列
    const vector<string> &get_actions()
    {
        return _actions;
    }

    // 返回当前时间片的值，返回值意味着所处在哪个时间片，你只能往这个时间片增加、删除动作。
    int get_current_time()
    {
        return current_index;
    }

    //手动让当前时间片向右移一步，返回移动后的时间片，如果超出总时间，则返回-1。
    int current_time_plus_one();

    // 返回指定时间片中，硬盘已经规划了的动作消耗的token数量,如果没有规划动作，则是0，如果索引越界，则返回-1。这个time可以指定任意时间片
    int get_action_tokens(int time);

    // 向硬盘当前时间片加入pass动作，默认在该时间片的string的末尾添加，若指定index，则在string的index处添加
    bool add_pass_action(int index = -1);

    // 向该硬盘当前时间片加入read动作，默认在该时间片的string数组的末尾添加，若指定index，则在index处添加
    bool add_read_action(int index = -1);

    // 向该硬盘当前时间片加入jump动作，加完后这个时间片就满了，但是该函数不会自动将当前时间片向右移动。
    bool add_jump_action(int distance);

    // 删除当前时间片的动作，默认删除当前时间片string的最后一个动作，若指定index，则删除当前时间片的string的index处的动作
    bool delete_action(int index = -1);

    // 删除当前时间片的所有动作，默认删除后不会将当前时间片向左移动，如果current_time_sub_one指定为true，则当前时间片向左移动一个单位
    bool delete_current_rime_actions(bool current_time_sub_one=false);
};

void Action_queue::check_end_index(int time)
{
    if (time = current_index - 1 && _actions[time].size() == 0)
    {
        current_index--;
        for (int i = time - 1; i >= 0; i--)
        {
            if (_actions[i].size() > 0)
            {
                break;
            }
            current_index--;
        }
    }
}

void Action_queue::check_back_read_tokens(int time)
{
    int count = time + 1;
    while (count < current_index && ((_actions[count].size() != 0 && _actions[count][0] == 'r') || _actions[count].size() == 0))
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


int Action_queue::current_time_plus_one()
    {
        if (current_index + 1 <= _actions.size())
        {
            return ++current_index;
        }
        else
        {
            return -1;
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

bool Action_queue::add_pass_action(int index)
{
    if(_tokens[current_index]>=G)
    {
        return false;
    }

    if (index)
    {
        _actions[current_index].insert(index, 1, 'p');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G,current_index,index);
        return true;
    }
    else
    {
        _actions[current_index].append("p");
        _tokens[current_index] += 1;
        return true;
    }
}

bool Action_queue::add_read_action(int index)
{
    if (index)
    {
        _actions[current_index].insert(index, 1, 'r');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, current_index, G, index);
        return true;
    }
    else
    {
        _actions[current_index].append("r");
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G,current_index, _actions.size()-1);
        return true;
    }
}

bool Action_queue::add_jump_action(int distance)
{
    if (_actions[current_index].size() > 0)
    {
        return -1;
    }
    else
    {
        _actions[current_index] = "j " + to_string(distance);
    }
    _tokens[current_index] = G;
}

bool Action_queue::delete_action(int index = -1)
{
    if (index)
    {
        if (_actions[current_index].size()==0||_actions[current_index][0] == 'j' || index < 0 || index >= _actions[current_index].size())
        {
            return false;
        }
        char s = _actions[current_index][index];
        auto begin = _actions[current_index].begin();
        _actions[current_index].erase(begin + index);
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, current_index);
    }
    else
    {
        if(_actions[current_index].size()==0)
        {
            return true;
        }
        if (_actions[current_index][0] == 'j')
        {
            _actions[current_index] = "";
            _tokens[current_index] = 0;
            return true;
        }
        char s = _actions[current_index][_actions.size() - 1];
        _actions[current_index].pop_back();
        if (s == 'p')
        {
            _tokens[current_index]--;
        }
        else
        {
            _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, current_index);
        }
    }

    return true;
}

bool Action_queue::delete_current_rime_actions(bool current_time_sub_one)
{
    _actions[current_index] = "";
    _tokens[current_index] = 0;
    if(current_time_sub_one&&current_index)
    {
        current_index--;
    }
    return true;
}