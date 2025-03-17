#include <iostream>
#include "Disk.hpp"
#include <vector>
#include "Object.hpp"
using namespace std;

#define MAX_DISK_NUM (10 + 1)
#define MAX_DISK_SIZE (16384 + 1)
#define MAX_REQUEST_NUM (30000000 + 1)
#define MAX_OBJECT_NUM (100000 + 1)
#define REP_NUM (3)
#define FRE_PER_SLICING (1800)
#define EXTRA_TIME (105)

class Controller
{
public:
    int num_T;          // 时间片数量
    int num_disk;       // 磁盘数量 3<=N<=10
    int num_tag;        // 标签数量
    int num_v;          // 每个硬盘的单元数量
    int G;              // 代表每个磁头每个时间片最多消耗的令牌数。输入数据保证64≤𝐺≤1000。
    vector<Disk> disks; // 存储硬盘的数组指针

    vector<int> num_delete_operation;
    vector<int> num_write_operation;
    vector<int> num_read_operation;

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
};