using namespace std;
#include<deque>
#include<string>
#include<vector>

class Strategy{
    public:

    //根据行为计算tokens的方式
    static int calculate_tokens(const string& actions, int G, int prev_token = 0) {
        int token_count = 0;
        for (size_t j = 0; j < actions.length(); j++) {
            if (actions[j] == 'Jump') {
                token_count += G;
            } else if (actions[j] == 'Pass') {
                token_count += 1;
            } else {
                if (j == 0) {
                    if (prev_token == 0) {
                        token_count += 64;
                    } else {
                        if (actions[actions.length()-1 ] != 'Read') {
                            token_count += 64;
                        } else {
                            token_count += max(16, static_cast<int>(prev_token * 0.8));
                        }
                    }
                } else {
                    if (actions[j - 1] != 'Read') {
                        token_count += 64;
                    } else {
                        token_count += max(16, static_cast<int>(prev_token * 0.8));
                    }
                }
            }
        }
        return token_count;
    }

    //输入的deque是双向循环队列，可以通过索引访问，索引的值是时间片的值
    //actions[i]存放的字符串代表时间片i中的动作序列。time是指定时间片。根据输入的actions，更新对应time时间片中动作序列消耗的token，若没有指定time，则更新所有时间片的token。tokens是更新之前记录的每个时间片消耗的token数量。
    //返回值：返回的tokens是更新过后的。
    static vector<int> recalculate_tokens(const vector<string>& actions, vector<int> tokens, int G, int time = -1) {
        if (time == -1) {
            // 更新所有时间片
            for (size_t i = 0; i < actions.size(); i++) {
                int prev_token = (i == 0) ? 0 : tokens[i - 1];
                tokens[i] += calculate_tokens(actions[i], G, prev_token);
            }
        } else {
            // 更新指定时间片
            if (time >= 0 && time < static_cast<int>(actions.size())) {
                int prev_token = (time == 0) ? 0 : tokens[time - 1];
                tokens[time] += calculate_tokens(actions[time], G, prev_token);
            }
        }
        return tokens;
    }
};