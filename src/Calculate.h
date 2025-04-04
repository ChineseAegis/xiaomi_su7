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
#include <thread>
#include <mutex>
#include <list>

#define REP_NUM (3)
struct ReadRequest;

class Calculate
{
    // 计算指定时间片、索引的某个r动作之前有几个r动作。
    static int calculate_num_pre_read_action(const vector<string> &actions, int time, int index);

    // 计算连续几步read所消耗的token
    static int computeValue(int base, double factor, int times);

    // 根据行为计算tokens的方式
    static int calculate_tokens(const string &actions, int G, const vector<string> &all_actions, int time);

    // 计算硬盘两点之间消耗的时间片
    static int cost_between_two_index(int head_index, int target_index, deque<int> read_queue_indexs, int current_time, int num_v, int G, int max_T);

    // 计算硬盘中两个索引之间的距离，num_v是该硬盘的索引总数
    static int distance_between_two_index(int begin_index, int end_index, int num_v);

public:
    // 更新对应时间片的token消耗
    // 输入的 actions 是时间片序列，每个时间片包含一个动作序列
    // tokens 是更新之前记录的每个时间片消耗的 token 数量
    // time 是指定时间片，若未指定，则更新所有时间片的 token
    // index如果指定，则代表all_actions[time][index]为新增动作，只需根据这个新增动作进行更新
    // 返回值：返回更新后的 tokens
    static void recalculate_tokens(const vector<string> &all_actions, vector<int> &tokens, int G, int time = -1, int index = -1, int num = 1);

    // 计算每个硬盘的block任务队列
    static void calculate_blocks_queue(
        const std::list<ReadRequest> &read_request_list,
        const std::unordered_map<int, Object> &objects,
        std::vector<Disk> &disks,
        std::vector<std::deque<int>> &disk_unread_indexs,
        int time,
        int num_v,
        int G,
        int num_T);

    static void append_blocks_for_new_requests(
        const std::vector<int> &new_request_ids,
        const std::unordered_map<int, std::list<ReadRequest>::iterator> &object_request_iters,
        const std::unordered_map<int, Object> &objects,
        std::vector<Disk> &disks,
        std::vector<std::deque<int>> &disk_unread_indexs,
        const unordered_map<int,vector<int>>& object_unread_ids,
        int num_v);

    // 已知一个硬盘的磁头位置，将要读取的所有块的索引，修改传入的action_queue，增加动作到相应时间片中。num v是每个硬盘的存储单元数量
    static int calculate_actions(int head_index, deque<int> &read_queue_indexs, Action_queue &action_queue, int current_time, int num_v, int G, bool is_continue = false);

    static deque<int> sort_unread_indexs(int head, const deque<int> &read_queue_indexs, int num_v, int n);
};
