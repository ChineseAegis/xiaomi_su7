using namespace std;
#include<deque>
#include<string>
#include<vector>

class Strategy{
    public:

    //输入的deque是双向循环队列，可以通过索引访问，索引的值是时间片的值
    //actions[i]存放的字符串代表时间片i中的动作序列。time是指定时间片。根据输入的actions，更新对应time时间片中动作序列消耗的token，若没有指定time，则更新所有时间片的token。tokens是更新之前记录的每个时间片消耗的token数量。
    //返回值：返回的tokens是更新过后的。
    static vector<int> recalculate_tokens(const vector<string>& actions,vector<int> tokens,int G,int time=-1){

        
        return tokens;
    }

};