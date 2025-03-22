#pragma once
#include <iostream>
#include "Disk.h"
#include <vector>
#include "Object.hpp"
#include <queue>
#include <string>
#include <list>
#include <deque>
#include "Calculate.h"
#include <unordered_map>
#include <unordered_set>
#include "Action_queue.h"

using namespace std;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)

class Action_queue;

struct WriteResult
{
    vector<int> id;             // 每个磁盘的id
    vector<vector<int>> indexs; // 对应每个磁盘中blocks位置的索引
    WriteResult(vector<int> id, vector<vector<int>> indexs) : id(id), indexs(indexs)
    {
    }
};

class Controller
{

public:
    int num_T;            // 时间片数量
    int num_disk;         // 磁盘数量 3<=N<=10
    int num_tag;          // 标签数量
    int num_v;            // 每个硬盘的单元数量
    int G;                // 代表每个磁头每个时间片最多消耗的令牌数。输入数据保证64≤𝐺≤1000。
    int current_time = 0; // 当前时间片
    vector<Disk> disks;   // 存储硬盘的数组

    unordered_map<int, Object> objects; // 存储记录所有对象

    unordered_map<int,vector<int>> object_unread_ids; // 存储已经请求读取，但还没有读取的对象,第一个int是对象id，第二个数组是存放请求id的数组

    int request_success_num=0;//读取成功但还没有上报的请求的个数
    unordered_map<int,vector<int>> object_read_sucess_ids;   // 存储已经读取成功，但还没有上报的对象,第一个int是对象id，第二个数组是存放请求id的数组
    

    // 记录每个阶段的该类指令涉及的块总大小
    vector<int> num_delete_operation;
    vector<int> num_write_operation;
    vector<int> num_read_operation;

    vector<Action_queue> disk_actions; // 硬盘指令队列，总共N个硬盘，就有N个队列，每个元素是一个时间片下所有指令的字符串

    Controller()
    {
    }

    // 将文件写入，并返回结果，结果中有写入位置的详细信息，object_id是对象id，size是对象大小，tag是对象标签
    WriteResult write_object_to_disk(int object_id, int size, int tag, vector<vector<Block *>> &p_blocks);

    bool delete_object_from_disk(int object_id);

    // 将block写入指定磁盘的指定位置，object_id指block属于哪个文件，id指的是block是该文件的第几个block。
    Block *write_block_to_disk(int disk_id, int index, int object_id, int id);

    //将读写请求存储到map
    void deal_read_request(int object_id,int request_id);

    void global_pre_proccess();

    void timestamp_action();

    void delete_action();

    void write_action();

    void read_action();

    void run();
};

