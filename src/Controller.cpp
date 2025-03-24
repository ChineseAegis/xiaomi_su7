#include "Controller.h"
#include <chrono>
#include <fstream>

void Controller::global_pre_proccess()
{
    scanf("%d%d%d%d%d", &num_T, &num_tag, &num_disk, &num_v, &G);

    for (int i = 0; i < num_disk; i++)
    {
        disks.push_back(Disk(num_v, i));
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%d", &num);
            num_delete_operation.push_back(num);
        }
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%d", &num);
            num_write_operation.push_back(num);
        }
    }

    for (int i = 1; i <= num_tag; i++)
    {
        for (int j = 1; j <= (num_T - 1) / FRE_PER_SLICING + 1; j++)
        {
            int num;
            scanf("%d", &num);
            num_read_operation.push_back(num);
        }
    }
    num_T += EXTRA_TIME;
    printf("OK\n");
    fflush(stdout);

    disk_actions.resize(num_disk, Action_queue(num_T, G));
    block_read_queue.resize(num_disk);
    disk_last_head_indexs.resize(num_disk, -1);
}

void Controller::timestamp_action()
{
    int timestamp;
    scanf("%*s%d", &timestamp);
    printf("TIMESTAMP %d\n", timestamp);

    fflush(stdout);
}

void Controller::delete_action()
{
    int n_delete;
    int abort_num = 0;
    vector<int> delete_ids;

    vector<pair<int, vector<int>>> object_read_failed_ids; // 存储读取失败的请求,第一个int是对象id，第二个数组是存放请求id的数组

    scanf("%d", &n_delete);
    for (int i = 0; i < n_delete; i++)
    {
        int delete_id;
        scanf("%d", &delete_id);
        delete_object_from_disk(delete_id - 1);
        if (object_unread_ids.find(delete_id - 1) != object_unread_ids.end())
        {
            vector<int> request_ids = object_unread_ids.find(delete_id - 1)->second;
            object_read_failed_ids.push_back(make_pair(delete_id - 1, request_ids));
            object_unread_ids.erase(delete_id - 1);
            abort_num += request_ids.size();
            for (auto &request_id : request_ids)
            {
                object_read_requests.erase(request_id);
                object_unread_requestid_block_count.erase(request_id);
            }
        }
    }

    printf("%d\n", abort_num);

    for (int i = 0; i < object_read_failed_ids.size(); i++)
    {
        for (auto &id : object_read_failed_ids[i].second)
        {
            printf("%d\n", id + 1);
        }
    }
    //object_read_failed_ids.clear();
    if (abort_num > 0)
    {
        vector<int> temp;
        new_request_ids.swap(temp);
        for(auto& it:object_read_requests)
        {
            new_request_ids.push_back(it.first);
        }
        this->calculate_actions();
    }
    fflush(stdout);
}

void Controller::write_action()
{
    int n_write;
    scanf("%d", &n_write);
    for (int i = 0; i < n_write; i++)
    {
        int id, size, tag;
        scanf("%d%d%d", &id, &size, &tag);

        vector<vector<Block *>> p_blocks;

        WriteResult result = write_object_to_disk(id - 1, size, tag, p_blocks);

        objects.insert(make_pair(id - 1, Object(id - 1, size, tag, p_blocks)));
        printf("%d\n", id);
        for (int j = 0; j < REP_NUM; j++)
        {
            int disk_id = result.id[j];
            printf("%d", disk_id + 1);
            for (int k = 0; k < size; k++)
            {
                int index = result.indexs[j][k];
                printf(" %d", index + 1);
            }
            printf("\n");
        }
    }

    fflush(stdout);
}

void Controller::read_action()
{
    int n_read;
    int request_id, object_id;
    scanf("%d", &n_read);
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < n_read; i++)
    {
        scanf("%d%d", &request_id, &object_id);
        deal_read_request(object_id - 1, request_id - 1);
    }

    // 统计 calculate_actions 的耗时（毫秒）

    if (n_read > 0)
    {
        this->calculate_actions();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::ofstream log("action_times.log", std::ios::app);
    log << "[Time]  this->calculate_actions(): " << duration << " ms\n";
    log.close();

    for (int i = 0; i < disk_actions.size(); i++)
    {
        while (disk_actions[i].get_current_time() <= current_time && block_read_queue[i].size() > 0)
        {
            int head_index;
            if (disk_last_head_indexs[i] == -1)
            {
                head_index = disks[i].head;
            }
            else
            {
                head_index = disk_last_head_indexs[i];
            }
            block_read_queue[i] = Calculate::sort_unread_indexs(head_index, block_read_queue[i], num_v,5);
            disk_last_head_indexs[i] = Calculate::calculate_actions(head_index, block_read_queue[i], this->disk_actions[i], current_time, num_v, G, true);
        }
    }
   
    // 输出动作
    for (int i = 0; i < disk_actions.size(); i++)
    {
        const vector<string> &actions = disk_actions[i].get_actions();
        const string &orig_s = actions[current_time]; // 引用，避免复制

        this->execute_actions(i, orig_s); // 如果 execute_actions 不会修改 orig_s，可以直接传引用

        string s; // 只在需要时才创建副本
        if (orig_s.empty())
        {
            s = "#";
        }
        else if (orig_s[0] == 'j')
        {
            int len = orig_s.size() - 2;
            string sub_str = orig_s.substr(2, len);
            int target_index = stoi(sub_str) + 1;
            s = "j " + to_string(target_index);
        }
        else
        {
            s = orig_s + "#";
        }

        printf("%s\n", s.c_str());
    }
    
    printf("%d\n", this->request_success_num);
    for (auto &request_id : this->object_read_sucess_ids)
    {
        printf("%d\n", request_id + 1);
    }
    this->request_success_num = 0;
    std::vector<int> temp;
    object_read_sucess_ids.swap(temp);

    fflush(stdout);
}

void Controller::run()
{
    timestamp_action();

    delete_action();

    write_action();


    read_action();

    current_time++;
}

Block *Controller::write_block_to_disk(int disk_id, int index, int object_id, int id)
{
    if (index < 0 || index >= num_v)
    {
        return nullptr;
    }
    if (disks[disk_id].units[index] != nullptr)
    {
        return nullptr;
    }
    disks[disk_id].units[index] = new Block(disk_id, index, object_id, id);
    disks[disk_id].num_free_unit--;
    return disks[disk_id].units[index];
}
WriteResult Controller::write_object_to_disk(int object_id, int size, int tag, vector<vector<Block *>> &p_blocks)
{
    p_blocks.resize(REP_NUM);
    for (int i = 0; i < REP_NUM; i++)
    {
        p_blocks[i].resize(size);
    }
    vector<pair<int, int>> disk_pair_ids;
    for (int i = 0; i < disks.size(); i++)
    {
        disk_pair_ids.push_back(make_pair(disks[i].id, disks[i].num_free_unit));
    }

    sort(disk_pair_ids.begin(), disk_pair_ids.end(), [](const pair<int, int> &p1, const pair<int, int> &p2)
         { return p1.second > p2.second; });

    vector<vector<int>> indexs;
    indexs.resize(REP_NUM);
    for (int i = 0; i < indexs.size(); i++)
    {
        indexs[i].resize(size);
    }

    for (int i = 0; i < REP_NUM; i++)
    {
        int disk_id = disk_pair_ids[i].first;
        int count = size;
        for (int j = 0; j < num_v; j++)
        {
            if (!count)
            {
                break;
            }
            // throw std::runtime_error(to_string(disk_id));
            if (disks[disk_id].units[j] == nullptr)
            {
                indexs[i][size - count] = j;
                p_blocks[i][size - count] = write_block_to_disk(disk_id, j, object_id, size - count);
                count--;
            }
        }
    }

    vector<int> disk_ids;

    for (auto &d : disk_pair_ids)
    {
        disk_ids.push_back(d.first);
    }

    return WriteResult(disk_ids, indexs);
}

bool Controller::delete_object_from_disk(int object_id)
{
    if (objects.find(object_id) == objects.end())
    {
        return false;
    }
    Object obj = objects.at(object_id);
    for (int i = 0; i < obj.blocks.size(); i++)
    {
        vector<Block *> p_blocks = obj.blocks[i];
        for (int j = 0; j < p_blocks.size(); j++)
        {
            int disk_id = p_blocks[j]->disk_id;
            int index = p_blocks[j]->index;
            Block *p = disks[disk_id].units[index];
            if (p)
            {
                delete p;
                p = nullptr;
            }
            disks[disk_id].units[index] = nullptr;
            disks[disk_id].num_free_unit++;
        }
    }
    objects.erase(object_id);
    return true;
}

void Controller::deal_read_request(int object_id, int request_id)
{

        object_unread_ids[object_id].push_back(request_id);

    vector<int> temp;
    int n = objects.at(object_id).size;
    for (int i = 0; i < n; i++)
    {
        temp.push_back(0);
    }
    object_read_requests.insert(make_pair(request_id, make_pair(object_id, temp)));
    object_unread_requestid_block_count[request_id] = 0;
    new_request_ids.push_back(request_id);
}

// void Controller::calculate_actions_process_index(int start, int end)
// {
//     for (int i = start; i < end; ++i)
//     {
//         block_read_queue[i] = Calculate::sort_unread_indexs(disks[i].head, block_read_queue[i], num_v,block_read_queue[i].size()>5?5:block_read_queue[i].size());
//         disk_last_head_indexs[i] = Calculate::calculate_actions(disks[i].head, block_read_queue[i], disk_actions[i], current_time, num_v, G);
//     }
// }

void Controller::calculate_actions()
{
    auto start = std::chrono::high_resolution_clock::now();
    Calculate::calculate_blocks_queue(object_read_requests,new_request_ids, objects, disks, block_read_queue, current_time, num_v, G, num_T);

    vector<int> temp;
    new_request_ids.swap(temp);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::ofstream log("action_times.log", std::ios::app);
    log << "[Time] block_read_queue: " << duration << " ms\n";
    log.close();
    for (int i = 0; i < block_read_queue.size(); i++)
    {
        block_read_queue[i] = Calculate::sort_unread_indexs(disks[i].head, block_read_queue[i], num_v,5);
        disk_last_head_indexs[i] = Calculate::calculate_actions(disks[i].head, block_read_queue[i], this->disk_actions[i], current_time, num_v, G);
    }
    
    // int total = block_read_queue.size();
    // int thread_count = 4;
    // int batch_size = std::ceil(total / static_cast<float>(thread_count));

    // std::vector<std::thread> threads;

    // for (int t = 0; t < thread_count; ++t) {
    //     int start = t * batch_size;
    //     int end = std::min(start + batch_size, total); // 防止越界
    //     if (start >= total) break; // 多线程数多于任务数时避免空线程
    //     threads.emplace_back(calculate_actions_process_index, start, end);
    // }

    // for (auto& t : threads) {
    //     t.join();
    // }
}

void Controller::execute_actions(int disk_id, const string &action)
{
    if (action.size() > 0 && action[0] == 'j')
    {
        disks[disk_id].head = std::stoi(action.substr(2));
    }
    else
    {
        for (const char &c : action)
        {
            if (c == 'p')
            {
                disks[disk_id].head = (disks[disk_id].head + 1) % num_v;
            }
            else if (c == 'r')
            {
                Block *block = disks[disk_id].units[disks[disk_id].head];
                // if (current_time == 6 && block == nullptr)
                // {
                //     throw runtime_error(action+"  "+to_string(disk_id)+":"+to_string(disks[disk_id].head));
                // }

                // if (object_unread_ids.find(block->object_id) != object_unread_ids.end())
                // {
                    vector<int> &request_ids = object_unread_ids.at(block->object_id);
                    for (auto &request_id : request_ids)
                    {
                        if (object_read_requests.at(request_id).second[block->id] == 0)
                        {
                            object_unread_requestid_block_count.at(request_id)++;
                            object_read_requests.at(request_id).second[block->id] = 1;
                        }

                        if (object_unread_requestid_block_count.at(request_id) == objects.at(block->object_id).size)
                        {
                            object_read_sucess_ids.push_back(request_id);
                            this->request_success_num += 1;
                            object_unread_requestid_block_count.erase(request_id);
                            object_read_requests.erase(request_id);
                        }
                    }
                    request_ids.erase(
                        std::remove_if(
                            request_ids.begin(),
                            request_ids.end(),
                            [&](int id)
                            {
                                return object_read_requests.find(id) == object_read_requests.end();
                            }),
                        request_ids.end());
                // }

                disks[disk_id].head = (disks[disk_id].head + 1) % num_v;
            }
        }
    }
}