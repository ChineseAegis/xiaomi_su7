#include<iostream>
#include<vector>
using namespace std;


//对象块
struct Block{
    int size=1;//对象块大小,当前默认等于1,不用修改
    int disk_id;//对象块所属的硬盘id
    int index;//对象块在硬盘中的位置，从0开始
    
    Block(int disk_id,int index):disk_id(disk_id),index(index){

    }
};



//硬盘
class Disk{
    public:
    int id;//唯一,从0开始
    int num_v;//硬盘存储单元数量,也表示最多可以存储的对象块的数量
    vector<Block*> storageUnits;//当前硬盘的存储单元情况,nullptr表示当前单元格空闲,否则指向具体存放的block
    int head=0;//磁头位置，从0开始

    Disk(int num_v):num_v(num_v)
    {
        for(int i=0;i<num_v;i++)
        {
            storageUnits.push_back(nullptr);
        }
    }

    
    
};