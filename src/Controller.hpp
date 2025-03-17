#include <iostream>
#include "Disk.hpp"
#include <vector>
#include "Object.hpp"
#include <queue>
#include <string>
#include <list>
#include <deque>
#include "Strategy.hpp"

using namespace std;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)

// 表示一个硬盘规划的未来若干个时间片的动作序列
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

    vector<string> &get_actions()
    {
        return _actions;
    }

    // 返回指定时间片中，硬盘已经规划了的动作所消耗的token数量
    int get_action_tokens(int time)
    {
        if (time >= _actions.size())
        {
            return -1;
        }
        return _tokens[time];
    }

    // 向该磁盘指定时间片加入pass动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_pass_action(int time, int index = -1)
    {
        if (time >= _actions.size())
        {
            return -1;
        }
        if (index)
        {
            _actions[time].insert(index, 1, 'p');
        }
        else
        {
            _actions[time].append("p");
        }
        _tokens = Strategy::recalculate_tokens(_actions, _tokens, time, G);
        return true;
    }
    // 向该磁盘指定时间片加入read动作，默认在指定时间片的指令序列的末尾添加，若指定index，则在index处添加
    bool add_read_action(int time, int index = -1)
    {
        if (time >= _actions.size())
        {
            return -1;
        }
        if (index)
        {
            _actions[time].insert(index, 1, 'r');
        }
        else
        {
            _actions[time].append("r");
        }
        _tokens = Strategy::recalculate_tokens(_actions, _tokens, time, G);
        return true;
    }

    bool add_jump_action(int time,int distance)
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
            _actions[time]="j "+to_string(distance);
        }
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

    vector<int> num_delete_operation;
    vector<int> num_write_operation;
    vector<int> num_read_operation;

    vector<Action_queue> disk_actions; // 硬盘指令队列，总共N个硬盘，就有N个队列，每个元素是一个时间片下所有指令的字符串

    Controller()
    {

    }

    void global_pre_proccess()
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

    void timestamp_action()
    {
        int timestamp;
        scanf("%*s%d", &timestamp);
        printf("TIMESTAMP %d\n", timestamp);

        fflush(stdout);
    }

    void delete_action()
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

    void write_action()
    {
        int n_write;
        scanf("%d", &n_write);
        for (int i = 1; i <= n_write; i++)
        {
            int id, size, tag;
            scanf("%d%d%*d", &id, &size, &tag);

            printf("%d\n", id);
            for (int j = 1; j <= REP_NUM; j++)
            {
                printf("%d", " ");
                for (int k = 1; k <= size; k++)
                {
                    printf(" %d", " ");
                }
                printf("\n");
            }
        }

        fflush(stdout);
    }

    void read_action()
    {
        int n_read;
        int request_id, object_id;
        scanf("%d", &n_read);
        for (int i = 1; i <= n_read; i++)
        {
            scanf("%d%d", &request_id, &object_id);
        }

        static int current_request = 0;
        static int current_phase = 0;
        if (!current_request && n_read > 0)
        {
            current_request = request_id;
        }
        if (!current_request)
        {
            for (int i = 1; i <= num_disk; i++)
            {
                printf("#\n");
            }
            printf("0\n");
        }

        fflush(stdout);
    }

    void run()
    {
    }
};