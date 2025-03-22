#include "Action_queue.h"

void Action_queue::check_end_index(int time)
{
    if (time == current_index - 1 && _actions[time].size() == 0)
    {
        current_index--;
        for (int i = time - 1; i >= 0; i--)
        {
            if (_actions[i].size() > 0)
            {
                break;
            }
            current_index--;
        }
    }
}

void Action_queue::check_back_read_tokens(int time)
{
    int count = time + 1;
    while (count < current_index && ((_actions[count].size() != 0 && _actions[count][0] == 'r') || _actions[count].size() == 0))
    {
        if (_actions[count].size() == 0)
        {
            count++;
            continue;
        }
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G, count);
        if (!all_of(_actions[count].begin(), _actions[count].end(), [](char c)
                    { return c == 'r'; }))
        {
            break;
        }
        count++;
    }
}


int Action_queue::current_time_plus_one()
    {
        if (current_index + 1 <= _actions.size())
        {
            return ++current_index;
        }
        else
        {
            return -1;
        }
    }
int Action_queue::get_action_tokens(int time)
{
    if (time >= _actions.size())
    {
        return -1;
    }
    return _tokens[time];
}

int Action_queue::add_string_to_current_action(string action)
{
    _actions[current_index]=action;
    _tokens=Calculate::recalculate_tokens(_actions,_tokens,G,current_index);
    int token = _tokens[current_index];
    if(token>G)
    {
        _actions[current_index]="";
        _tokens[current_index]=0;
        return token-G;
    }else
    {
        return 0;
    }
}

int Action_queue::add_pass_action(int num,int index)
{
    int t_token=_tokens[current_index];
    if (index>=0)
    {
        auto it = _actions[current_index].begin()+index;
        _actions[current_index].insert(it, num, 'p');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G,current_index,index);
    }
    else
    {
        auto it = _actions[current_index].end();
        _actions[current_index].insert(it, num, 'p');
        _tokens[current_index] += num;
    }
    int token = _tokens[current_index];
    if(token>G)
    {
        this->delete_action(index,index+num);
        _tokens[current_index]=t_token;
        return token-G;
    }else
    {
        return 0;
    }
}

int Action_queue::add_read_action(int num,int index)
{
    int t_token=_tokens[current_index];
    if (index)
    {
        auto it = _actions[current_index].begin()+index;
        _actions[current_index].insert(it, num, 'r');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, current_index, G, index);
        return true;
    }
    else
    {
        auto it = _actions[current_index].end();
        _actions[current_index].insert(it, num, 'r');
        _tokens = Calculate::recalculate_tokens(_actions, _tokens, G,current_index, _actions.size()-1);
        return true;
    }
}

bool Action_queue::add_jump_action(int distance)
{
    if (_actions[current_index].size() > 0)
    {
        return -1;
    }
    else
    {
        _actions[current_index] = "j " + to_string(distance);
    }
    _tokens[current_index] = G;
}

bool Action_queue::delete_action(int begin,int end)
{
    auto begin_it=_actions[current_index].begin()+begin;
    auto end_it=_actions[current_index].begin()+begin+end;
    _actions[current_index].erase(begin_it,end_it);
    _tokens=Calculate::recalculate_tokens(_actions,_tokens,G,current_index);
    return true;
}

bool Action_queue::delete_last_action(){
    if (_actions[current_index].empty()){
        return false;
    }
    _actions[current_index].pop_back();
    _tokens=Calculate::recalculate_tokens(_actions,_tokens,G,current_index);
    return true;
}

int Action_queue::current_time_sub_one(bool current_time_sub_one)
{
    _actions[current_index] = "";
    _tokens[current_index] = 0;
    if(current_time_sub_one&&current_index)
    {
        current_index--;
    }
    return current_index;
}

