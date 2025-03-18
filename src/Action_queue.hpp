#pragma once
#include <iostream>
#include <vector>
#include "Object.hpp"
#include <string>


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