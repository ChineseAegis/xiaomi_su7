#pragma once
#include <cmath> // 引入标准库中的 ceil 函数
using namespace std;
#include <deque>
#include <string>
#include <vector>
#include <algorithm> // 用于max函数
#include <stdexcept>
#include "Controller.h"
#include "Action_queue.h"
#include <unordered_set>
#include <unordered_map>
#include "Block.hpp"
#include "Disk.h"
#include <climits>

#define REP_NUM (3)

class Calculate
{
    // 计算指定时间片、索引的某个r动作之前有几个r动作。
    static int calculate_num_pre_read_action(const vector<string> &actions, int time, int index);

    // 计算连续几步read所消耗的token
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
    static vector<int> recalculate_tokens(const vector<string> &all_actions, vector<int> tokens, int G, int time = -1, int index = -1, int num = 1);

    // 已知一个硬盘的磁头位置，将要读取的所有块的索引，修改传入的action_queue，增加动作到相应时间片中。num v是每个硬盘的存储单元数量
    static void calculate_actions(int head_index, vector<int> read_queue_indexs, Action_queue &action_queue, int current_time, int num_v, int G);

    // 计算每个硬盘的block任务队列
    static vector<vector<int>> calculate_blocks_queue(unordered_map<int, Object> &object_unread_ids, vector<Disk> &disks, int time, int num_v, int G, int num_T);

    // 计算硬盘两点之间消耗的时间片
    static int cost_between_two_index(int head_index, int target_index, vector<int> read_queue_indexs, int current_time, int num_v, int G, int max_T);

    // 计算硬盘中两个索引之间的距离，num_v是该硬盘的索引总数
    static int distance_between_two_index(int begin_index, int end_index, int num_v);

    static vector<int> sort_unread_indexs(int head, vector<int> read_queue_indexs, int num_v);
};

