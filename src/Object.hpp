#pragma once
#include"Disk.hpp"
#include<vector>


//理解为文件
class Object{
    public:
    int id;//对象id,从0开始,唯一
    int size;//对象大小,数量是几就代表要分成几个对象块
    int tag;
    int num_copy=3;
    vector<vector<Block*>> blocks;//文件分为的对象块,二维数组,维度3 x size,指向具体的指针对象

    Object(int id,int size,int tag,vector<vector<Block*>> blocks):id(id),size(size),tag(tag),blocks(blocks)
    {
        
    }
};

struct ObjectHash {
    std::size_t operator()(const Object& p) const {
        return std::hash<int>()(p.id);  // 仅使用 id 计算哈希值
    }
};