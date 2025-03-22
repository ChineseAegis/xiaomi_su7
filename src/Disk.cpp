#include"Disk.h"

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