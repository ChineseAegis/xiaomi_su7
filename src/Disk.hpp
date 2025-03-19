#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include"Block.hpp"
using namespace std;



// 硬盘
class Disk
{
public:
    int id;                       // 唯一,从0开始
    int num_v;                    // 硬盘存储单元数量,也表示最多可以存储的对象块的数量
    vector<Block *> units; // 当前硬盘的存储单元情况,nullptr表示当前单元格空闲,否则指向具体存放的block
    int head = 0;                 // 磁头位置，从0开始
    int num_free_unit;
    vector<pair<int, int>> free_units;//记录空闲存储单元的数组，二元组第一个元素是空闲地址的起始位置，第二个元素是空闲存储单元长度

    Disk(int num_v,int id) : num_v(num_v), num_free_unit(num_v),id(id)
    {
        for (int i = 0; i < num_v; i++)
        {
            units.push_back(nullptr);
        }
        free_units.push_back(make_pair(0, num_v));
    }

    //查询索引所处第几个空闲段
    int get_block_in_which_free_unit(int index);

    //将两个存储单元的所有空闲段合并为一个空闲端，前提是索引直接的存储单元都是空闲的
    bool merge_two_free_units(int begin, int end);
};

int Disk::get_block_in_which_free_unit(int index)
{
    if(units[index]){
        return -1;
    }
    for (int i = 0; i < free_units.size(); i++)
    {
        if (i >= free_units[i].first && i < free_units[i].second + free_units[i].first)
        {
            return i;
        }
    }
}

bool Disk::merge_two_free_units(int begin, int end)
    {
        if(begin>end)return false;
        for(int i = begin; i < end + 1; i++){
            if(units[i]){
                return false;
            }
        }
        int first = get_block_in_which_free_unit(begin);
        int second = get_block_in_which_free_unit(end);

        int length = free_units[second].first+free_units[second].second;
        free_units[first].second+=length;
        auto i1 = free_units.begin()+first+1;
        auto i2 = free_units.begin()+second+1;
        free_units.erase(i1,i2);
        return true;

    }

