#include <iostream>
#include <vector>
#include <utility>
using namespace std;

// 对象块
struct Block
{
    int size = 1; // 对象块大小,当前默认等于1,不用修改

    int disk_id;   // 对象块所属的硬盘id
    int index;     // 对象块在硬盘中的位置，从0开始
    int object_id; // block所属的object id

    Block(int disk_id, int index, int object_id) : disk_id(disk_id), index(index), object_id(object_id)
    {
    }
};

// 硬盘
class Disk
{
public:
    int id;                       // 唯一,从0开始
    int num_v;                    // 硬盘存储单元数量,也表示最多可以存储的对象块的数量
    vector<Block *> storageUnits; // 当前硬盘的存储单元情况,nullptr表示当前单元格空闲,否则指向具体存放的block
    int head = 0;                 // 磁头位置，从0开始
    int num_free_unit;
    vector<pair<int, int>> free_units;

    Disk(int num_v) : num_v(num_v), num_free_unit(num_v)
    {
        for (int i = 0; i < num_v; i++)
        {
            storageUnits.push_back(nullptr);
        }
        free_units.push_back(make_pair(0, num_v));
    }

    int get_block_in_which_free_unit(int index)
    {
        for (int i = 0; i < free_units.size(); i++)
        {
            if (i >= free_units[i].first && i < free_units[i].second + free_units[i].first)
            {
                return i;
            }
        }
    }

    bool merge_two_free_units(int begin, int end)
    {
        if(begin>end)return false;
        for(int i = begin; i < end + 1; i++){

        }
        int first = get_block_in_which_free_unit(begin);
        int second = get_block_in_which_free_unit(end);

    }
};