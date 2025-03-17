#include <cmath> // 引入标准库中的 ceil 函数
using namespace std;
#include<deque>
#include<string>
#include<vector>
#include<algorithm> // 用于max函数
#include <stdexcept>

class Strategy {
public:
    // 根据行为计算tokens的方式
    static int calculate_tokens(const string& actions, int G, int prev_token = 0) {
        int token_count = 0;
        bool has_jed = false; // 标记是否已经执行了j动作

        for (size_t j = 0; j < actions.length(); j++) {
            if (actions[j] == 'j') {
                if (j != 0) {
                    // j只能在时间片的开始执行
                    throw invalid_argument("j action can only be executed at the beginning of a time slice.");
                }
                token_count += G;
                has_jed = true;
                // j执行后，该时间片不能有其他动作
                if (actions.length() > 1) {
                    throw invalid_argument("No other actions are allowed after j in the same time slice.");
                }
            } else if (actions[j] == 'p') {
                if (has_jed) {
                    throw invalid_argument("No other actions are allowed after j in the same time slice.");
                }
                token_count += 1;
            } else if (actions[j] == 'r') {
                if (has_jed) {
                    throw invalid_argument("No other actions are allowed after j in the same time slice.");
                }
                if (j == 0) {
                    // 时间片的第一个r动作
                    if (prev_token == 0) {
                        token_count += 64;
                    } else {
                        token_count += max(16, static_cast<int>(ceil(prev_token * 0.8)));
                    }
                } else {
                    if (actions[j - 1] != 'r') {
                        token_count += 64;
                    } else {
                        token_count += max(16, static_cast<int>(ceil(prev_token * 0.8)));
                    }
                }
            } else {
                throw invalid_argument("Invalid action: " + string(1, actions[j]));
            }
        }
        return token_count;
    }

    // 更新对应时间片的token消耗
    //输入的deque是双向循环队列，可以通过索引访问，索引的值是时间片的值
   //actions[i]存放的字符串代表时间片i中的动作序列。time是指定时间片。根据输入的actions，更新对应time时间片中动作序列消耗的token，若没有指定time，则更新所有时间片的token。tokens是更新之前记录的每个时间片消耗的token数量。
   //返回值：返回的tokens是更新过后的。
    static vector<int> recalculate_tokens(const vector<string>& actions, vector<int> tokens, int G, int time = -1) {
        if (time == -1) {
            // 更新所有时间片
            for (size_t i = 0; i < actions.size(); i++) {
                int prev_token = (i == 0) ? 0 : tokens[i - 1];
                tokens[i] = calculate_tokens(actions[i], G, prev_token);
            }
        } else {
            // 更新指定时间片
            if (time >= 0 && time < static_cast<int>(actions.size())) {
                int prev_token = (time == 0) ? 0 : tokens[time - 1];
                tokens[time] = calculate_tokens(actions[time], G, prev_token);
            }
        }
        return tokens;
    }
};