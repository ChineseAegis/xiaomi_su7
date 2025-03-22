#pragma once
#include <iostream>
#include "Disk.hpp"
#include <vector>
#include "Object.hpp"
#include <queue>
#include <string>
#include <list>
#include <deque>
#include "Calculate.hpp"
#include <unordered_map>
#include <unordered_set>
#include "Action_queue.hpp"

using namespace std;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)

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

void Controller::global_pre_proccess()
{
    scanf("%d%d%d%d%d", &num_T, &num_tag, &num_disk, &num_v, &G);
    num_T+=EXTRA_TIME;
    for (int i = 0; i < num_disk; i++)
    {
        disks.push_back(Disk(num_v,i));
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

    disk_actions.resize(num_disk,Action_queue(num_T,G));
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
    vector<int> delete_ids;

    vector<pair<int,vector<int>>> object_read_failed_ids;//存储读取失败的请求,第一个int是对象id，第二个数组是存放请求id的数组

    scanf("%d", &n_delete);
    for (int i = 0; i < n_delete; i++)
    {
        int delete_id;
        scanf("%d", &delete_id);
        delete_object_from_disk(delete_id - 1);
        if (object_unread_ids.find(delete_id - 1) != object_unread_ids.end())
        {
            vector<int> request_ids = object_unread_ids.find(delete_id - 1)->second;
            object_read_failed_ids.push_back(make_pair(delete_id - 1,request_ids));
            object_unread_ids.erase(delete_id - 1);
            abort_num+=request_ids.size();
        }
    }

    printf("%d\n", abort_num);

    for (int i = 0; i < object_read_failed_ids.size(); i++)
    {
        for(auto& id : object_read_failed_ids[i].second)
        {
            printf("%d\n", id+1);
        }
        
    }
    object_read_failed_ids.clear();
    fflush(stdout);
}

void Controller::write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 0; i < n_write; i++)
    {
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);

        vector<vector<Block *>> p_blocks;

        WriteResult result = write_object_to_disk(id - 1, size, tag, p_blocks);

        objects.insert(make_pair(id - 1, Object(id - 1, size, tag, p_blocks)));
        printf("%d\n", id);
        for (int j = 0; j < REP_NUM; j++)
        {
            int disk_id = result.id[j];
            printf("%d", disk_id+1);
            for (int k = 0; k < size; k++)
            {
                int index = result.indexs[j][k];
                printf(" %d", index+1);
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
        deal_read_request(object_id-1,request_id-1);
    }

    // 输出动作
    for (int i = 0; i < disk_actions.size(); i++)
    {
        vector<string> actions = disk_actions[i].get_actions();
        string s = actions[current_time];
        if(s.size()>0&&s[0]!='j'){
            s += "#";
        }else if(s.size()==0)
        {
            s += "#";
        }
        const char *output = s.c_str();
        printf("%s\n", output);
    }


    printf("%d\n",this->request_success_num);

    for(auto& request:this->object_read_sucess_ids)
    {
        for(auto& request_id:request.second)
        {
            printf("%d\n",request_id+1);
        }
    }
    this->request_success_num=0;

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
    if (disks[disk_id].units[index]!=nullptr)
    {
        return nullptr;
    }
    disks[disk_id].units[index] = new Block(disk_id, index, object_id, id);
    disks[disk_id].num_free_unit--;
    return disks[disk_id].units[index];
}
WriteResult Controller::write_object_to_disk(int object_id, int size, int tag, vector<vector<Block *>> &p_blocks)
{
    p_blocks.resize(REP_NUM);
    for (int i = 0; i < REP_NUM; i++)
    {
        p_blocks[i].resize(size);
    }
    vector<pair<int, int>> disk_pair_ids;
    for (int i = 0; i < disks.size(); i++)
    {
        disk_pair_ids.push_back(make_pair(disks[i].id, disks[i].num_free_unit));
    }

    sort(disk_pair_ids.begin(), disk_pair_ids.end(), [](const pair<int, int> &p1, const pair<int, int> &p2)
         { return p1.second > p2.second; });

    vector<vector<int>> indexs;
    indexs.resize(REP_NUM);
    for (int i = 0; i < indexs.size(); i++)
    {
        indexs[i].resize(size);
    }
    
    for (int i = 0; i < REP_NUM; i++)
    {
        int disk_id = disk_pair_ids[i].first;
        int count = size;
        for (int j = 0; j < num_v; j++)
        {
            if (!count)
            {
                break;
            }        
            //throw std::runtime_error(to_string(disk_id));
            if (disks[disk_id].units[j]==nullptr)
            {
                indexs[i][size - count] = j;
                p_blocks[i][size - count] = write_block_to_disk(disk_id, j, object_id, size - count);
                count--;
            }
        }
    }

    vector<int> disk_ids;

    for (auto &d : disk_pair_ids)
    {
        disk_ids.push_back(d.first);
    }

    return WriteResult(disk_ids, indexs);
}

bool Controller::delete_object_from_disk(int object_id)
{
    if (objects.find(object_id) == objects.end())
    {
        return false;
    }
    Object obj = objects.at(object_id);
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

void Controller::deal_read_request(int object_id,int request_id)
{
    if(this->object_unread_ids.find(object_id)!=object_unread_ids.end())
    {
        object_unread_ids[object_id].push_back(request_id);
    }else{
        vector<int> request_ids;
        request_ids.push_back(request_id);
        object_unread_ids.insert(make_pair(object_id,request_ids));
    }
}