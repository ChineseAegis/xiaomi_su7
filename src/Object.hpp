#include"Disk.hpp"
#include<vector>

//理解为文件
class Object{
    int id;//对象id,从0开始,唯一
    int size;//对象大小,数量是几就代表要分成几个对象块
    int tag;
    int num_copy=3;
    vector<vector<Block*>> blocks;//文件分为的对象块,具体如何存放,二维数组,维度3 x size

    Object(int id,int size,int tag,vector<vector<Block*>> blocks):id(id),size(size),tag(tag),blocks(blocks)
    {
        
    }
};