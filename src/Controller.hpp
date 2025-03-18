#pragma once
#include <iostream>
#include "Disk.hpp"
#include <vector>
#include "Object.hpp"
#include <queue>
#include <string>
#include <list>
#include <deque>
#include "Strategy.hpp"
#include <unordered_map>

using namespace std;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)

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
    unordered_map<int, Object> objects;

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

    void global_pre_proccess();

    void timestamp_action();

    void delete_action();

    void write_action();

    void read_action();

    void run();
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

void Controller::global_pre_proccess()
{
    scanf("%d%d%d%d%d", &num_T, &num_tag, &num_disk, &num_v, &G);
    for (int i = 0; i < num_disk; i++)
    {
        disks.push_back(Disk(num_v));
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%*d", &num);
            num_delete_operation.push_back(num);
        }
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%*d", &num);
            num_write_operation.push_back(num);
        }
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%*d", &num);
            num_read_operation.push_back(num);
        }
    }

    printf("OK\n");
    fflush(stdout);

    disk_actions.resize(num_disk);
}

void Controller::timestamp_action()
{
    int timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);

    fflush(stdout);
}

void Controller::delete_action()
{
    int n_delete;
    int abort_num = 0;
    static int _id[MAX_OBJECT_NUM];

    scanf("%d", &n_delete);
    for (int i = 1; i <= n_delete; i++)
    {
        scanf("%d", &_id[i]);
    }

    printf("%d\n", abort_num);

    fflush(stdout);
}

void Controller::write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 1; i <= n_write; i++)
    {
        int id, size, tag;
        scanf("%d%d%*d", &id, &size, &tag);

        vector<vector<Block *>> p_blocks;
        p_blocks.resize(REP_NUM);
        for (int i = 0; i < REP_NUM; i++)
        {
            p_blocks[i].resize(size);
        }
        WriteResult result = write_object_to_disk(id - 1, size, tag, p_blocks);

        objects.insert(make_pair(id - 1, Object(id - 1, size, tag, p_blocks)));
        printf("%d\n", id);
        for (int j = 0; j < REP_NUM; j++)
        {
            int disk_id = result.id[j];
            printf("%d", disk_id);
            for (int k = 0; k < size; k++)
            {
                int index = result.indexs[i][k];
                printf(" %d", index);
            }
            printf("\n");
        }
    }

    fflush(stdout);
}

void Controller::read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    for (int i = 1; i <= n_read; i++)
    {
        scanf("%d%d", &request_id, &object_id);
    }

    // 输出动作
    for (int i = 0; i < disk_actions.size(); i++)
    {
        vector<string> actions = disk_actions[i].get_actions();
        string s = actions[current_time];
        s += "#";
        const char *output = s.c_str();
        printf("%s\n", output);
    }

    fflush(stdout);
}

void Controller::run()
{
    timestamp_action();
    delete_action();
    write_action();
    read_action();
    current_time++;
}

Block *Controller::write_block_to_disk(int disk_id, int index, int object_id, int id)
{
    if (index < 0 || index >= num_v)
    {
        return nullptr;
    }
    if (disks[disk_id].units[index])
    {
        return nullptr;
    }
    disks[disk_id].units[index] = new Block(disk_id, index, object_id, id);
    disks[disk_id].num_free_unit--;
    return disks[disk_id].units[index];
}
WriteResult Controller::write_object_to_disk(int object_id, int size, int tag, vector<vector<Block *>> &p_blocks)
{
    vector<int> disk_ids;
    for (int i = 0; i < disks.size(); i++)
    {
        disk_ids.push_back(disks[i].num_free_unit);
    }

    sort(disk_ids.begin(), disk_ids.end(), std::greater<int>());

    vector<vector<int>> indexs;
    indexs.resize(REP_NUM);
    for (int i = 0; i < indexs.size(); i++)
    {
        indexs[i].resize(size);
    }

    int count = size;
    for (int i = 0; i < REP_NUM; i++)
    {

        for (int j = 0; j < num_v; j++)
        {
            if (!count)
            {
                break;
            }
            int disk_id = disk_ids[i];
            if (!disks[disk_id].units[j])
            {
                indexs[i][size - count] = j;
                p_blocks[i][size - count] = write_block_to_disk(disk_id, j, object_id, size - count);
                count--;
            }
        }
    }

    return WriteResult(disk_ids, indexs);
}

bool Controller::delete_object_from_disk(int object_id)
{
    if (objects.find(object_id) == objects.end())
    {
        return false;
    }
    Object obj = objects[object_id];
    for (int i = 0; i < obj.blocks.size(); i++)
    {
        vector<Block *> p_blocks = obj.blocks[i];
        for (int j = 0; j < p_blocks.size(); j++)
        {
            int disk_id = p_blocks[j]->disk_id;
            int index = p_blocks[j]->index;
            Block *p = disks[disk_id].units[index];
            if (p)
            {
                delete p;
                p = nullptr;
            }
            disks[disk_id].units[index] = nullptr;
            disks[disk_id].num_free_unit++;
        }
    }
    objects.erase(object_id);
    return true;
}