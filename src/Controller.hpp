#include<iostream>
#include"Disk.hpp"
#include<vector>
using namespace std;
//yyq

class Controller{
    public:
    int num_T;//时间片数量
    int num_disk;//磁盘数量 3<=N<=10
    int num_tag;//标签数量
    int num_v;//每个硬盘的单元数量
    vector<Disk> disks; //存储硬盘的数组指针

    Controller(int num_T,int num_disk,int num_tag,int num_v):num_T(num_T),num_disk(num_disk),num_tag(num_tag)
    {
        for(int i=0;i<num_disk;i++)
        {
            disks.push_back(Disk(num_v));
        }
    }
};