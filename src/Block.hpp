#pragma once
#include <iostream>
#include <vector>
#include <utility>
using namespace std;

// 对象块
struct Block
{
    int size = 1; // 对象块大小,当前默认等于1,不用修改

    int id;//对象块id，代表该对象块是object的第几个块
    int disk_id;   // 对象块所属的硬盘id
    int index;     // 对象块在硬盘中的位置，从0开始
    int object_id; // block所属的object id

    Block(int disk_id, int index, int object_id,int id) : disk_id(disk_id), index(index), object_id(object_id),id(id)
    {
    }
};