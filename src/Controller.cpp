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
                auto it = object_request_iters.find(request_id);
                if (it != object_request_iters.end())
                {
                    read_request_list.erase(it->second);
                    object_request_iters.erase(it);
                }

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
    // object_read_failed_ids.clear();
    if (abort_num > 0)
    {
        this->calculate_actions(true);
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

        WriteResult result = write_object_to_disk(id - 1, size, tag-1, p_blocks);

        objects.insert(make_pair(id - 1, Object(id - 1, size, tag-1, p_blocks)));
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

    for (int i = 0; i < n_read; i++)
    {
        scanf("%d%d", &request_id, &object_id);
        deal_read_request(object_id - 1, request_id - 1);
    }

    // 统计 calculate_actions 的耗时（毫秒）


    if (n_read > 0&&frequence<=0)
    {
        
        frequence=read_frequence;
        this->calculate_actions();
    }

    if (n_read >30)
    {
    if(read_frequence<=50)
        {
            read_frequence++;
        }
    }else{
        if(read_frequence>0)
        {
            read_frequence--;
        }
    }
    frequence--;

    
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
            // block_read_queue[i] = Calculate::sort_unread_indexs(head_index, block_read_queue[i], num_v, 5);
            disk_last_head_indexs[i] = Calculate::calculate_actions(head_index, block_read_queue[i], this->disk_actions[i], current_time, num_v, G, true);
            if (disk_last_head_indexs[i] == head_index)
            {
                break;
            }
        }
    }

    // 输出动作
    // 提前分配空间，避免频繁的内存分配

    string s;
    s.reserve(500); // 根据实际需求调整预留空间

    for (int i = 0; i < disk_actions.size(); i++)
    {
        const vector<string> &actions = disk_actions[i].get_actions();
        const string &orig_s = actions[current_time]; // 引用，避免复制

        this->execute_actions(i, orig_s); // 如果 execute_actions 不会修改 orig_s，可以直接传引用

        if (orig_s.empty())
        {
            // 空字符串直接输出
            printf("#\n");
        }
        else if (orig_s[0] == 'j')
        {
            // 只需要计算一次 target_index
            int len = orig_s.size() - 2;
            string sub_str = orig_s.substr(2, len);
            int target_index = stoi(sub_str) + 1;
            printf("j %d\n", target_index);
        }
        else
        {
            // 对于非空的 orig_s，只在需要时拼接#
            s = orig_s + "#";
            printf("%s\n", s.c_str());
        }
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
    // auto start = std::chrono::high_resolution_clock::now();
    read_action();
    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // std::ofstream log("action_times.log", std::ios::app);
    // log << "[Time] read: " << duration << " ms\n";
    // log.close();
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

// Controller.cpp 实现文件
Partition& Controller::get_partition(int tag, int size) {
    const int SIZE_THRESHOLD = 1024; // 或从配置读取
    bool is_large = size > SIZE_THRESHOLD;
    
    // 遍历现有分区查找匹配
    for (auto& p : partitions) {
        if (p.tag == tag && p.is_large_object == is_large) 
            return p;
    }
    
    // 未找到则创建新分区（需补充创建逻辑）
    throw runtime_error("Partition not found"); 
    // 实际项目可在此处调用 create_partition(tag, is_large);
}

WriteResult Controller::write_object_to_disk(int object_id, int size, int tag, vector<vector<Block *>> &p_blocks)
{
    Partition &partition = get_partition(tag, size);
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

    //按空闲数量选择磁盘，负载均衡
    sort(disk_pair_ids.begin(), disk_pair_ids.end(), [](const pair<int, int> &p1, const pair<int, int> &p2)
         { return p1.second > p2.second; });

    vector<vector<int>> indexs;//表示每个副本中每个块在磁盘上的具体存储位置（单元索引）
    indexs.resize(REP_NUM);
    for (int i = 0; i < indexs.size(); i++)
    {
        indexs[i].resize(size);
    }

    for (int i = 0; i < REP_NUM;i++){
        int disk_id = disk_pair_ids[i].first;
        bool allocated = false;
    //判断是否具有连续的存储空间
        for (int j = 0; j <=disks[disk_id].units.size()-size;j++){
            bool is_continuous = true;
            for (int k = 0; k < size;k++){
                if(disks[disk_id].units[j+k]!=nullptr){
                    is_continuous = false;
                    break;
                }
            }

            if(is_continuous){
                for (int k = 0; k < size;k++){
                    indexs[i][k] = j + k;
                    p_blocks[i][k] = write_block_to_disk(disk_id, j + k, object_id, k);
                }
                allocated = true;
                break;
            }
        }

        if(!allocated){
            int count = size;
            for (int j = tag*((num_v)/num_tag); ; j=(j+1)%(num_v-1))
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
                if((j+1)%(num_v-1)==tag*(num_v/num_tag))
            {
                break;
            }
        }
        }
    }
        // for (int i = 0; i < REP_NUM; i++)
        // {
        //     int disk_id = disk_pair_ids[i].first;
        //     int count = size;
        //     for (int j = 0; j < num_v; j++)
        //     {
        //         if (!count)
        //         {
        //             break;
        //         }
        //         // throw std::runtime_error(to_string(disk_id));
        //         if (disks[disk_id].units[j] == nullptr)
        //         {
        //             indexs[i][size - count] = j;
        //             p_blocks[i][size - count] = write_block_to_disk(disk_id, j, object_id, size - count);
        //             count--;
        //         }
        //     }
        // }

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

    ReadRequest req;
    req.request_id = request_id;
    req.object_id = object_id;
    req.blocks = std::vector<int>(objects.at(object_id).size, 0);

    read_request_list.push_back(req);
    object_request_iters[request_id] = std::prev(read_request_list.end());

    object_unread_requestid_block_count[request_id] = 0;

    // ✅ 标记为“新增请求”
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

void Controller::calculate_actions(bool force_full)
{
    // auto start = std::chrono::high_resolution_clock::now();

    // if (force_full)
    // {
    // 全量重建 block_read_queue（恢复旧逻辑）
    Calculate::calculate_blocks_queue(
        this->read_request_list,
        this->objects,
        this->disks,
        this->block_read_queue,
        current_time,
        num_v,
        G,
        num_T);
    // }
    // else if (!new_request_ids.empty())
    // {

    //     // 增量更新
    //     Calculate::append_blocks_for_new_requests(
    //         this->new_request_ids,
    //         this->object_request_iters,
    //         this->objects,
    //         this->disks,
    //         this->block_read_queue,
    //         this->object_unread_ids,
    //         this->num_v);
    //     new_request_ids.clear();

    // }

    // auto end = std::chrono::high_resolution_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    // std::ofstream log("action_times.log", std::ios::app);
    // log << "[Time] calculate_actions(" << (force_full ? "full" : "incremental") << "): " << duration << " ms\n";
    // log.close();

    const int thread_count = 8;
    std::vector<std::thread> threads(thread_count);

    int total_disks = static_cast<int>(block_read_queue.size());
    int per_thread = (total_disks + thread_count - 1) / thread_count;

    for (int t = 0; t < thread_count; ++t)
    {
        threads[t] = std::thread([this, t, per_thread, total_disks]() {
            int start_idx = t * per_thread;
            int end_idx = std::min(start_idx + per_thread, total_disks);
            for (int i = start_idx; i < end_idx; ++i)
            {
                block_read_queue[i] = Calculate::sort_unread_indexs(disks[i].head, block_read_queue[i], num_v, 5);
                disk_last_head_indexs[i] = Calculate::calculate_actions(
                    disks[i].head, block_read_queue[i], disk_actions[i], current_time, num_v, G);
            }
        });
    }

    // 等待所有线程结束
    for (auto& th : threads)
    {
        if (th.joinable()) th.join();
    }

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
                if (block != nullptr)
                {
                    // if(current_time==34696&&block==nullptr)
                    // {
                    //     throw runtime_error("test");
                    // }
                    int object_id = block->object_id;
                    int block_id = block->id;

                    if (object_unread_ids.find(object_id) != object_unread_ids.end())
                    {
                        vector<int> &request_ids = object_unread_ids.at(object_id);
                        for (auto &request_id : request_ids)
                        {
                            auto it = object_request_iters.find(request_id);
                            if (it == object_request_iters.end())
                                continue;

                            ReadRequest &req = *(it->second);
                            if (req.blocks[block_id] == 0)
                            {
                                req.blocks[block_id] = 1;
                                object_unread_requestid_block_count[request_id]++;
                            }

                            if (object_unread_requestid_block_count[request_id] == req.blocks.size())
                            {
                                object_read_sucess_ids.push_back(request_id);
                                this->request_success_num++;

                                // 删除对应链表节点和映射
                                read_request_list.erase(it->second);
                                object_request_iters.erase(it);
                                object_unread_requestid_block_count.erase(request_id);
                            }
                        }

                        // 清理无效 request_id（成功读取后已删除）
                        request_ids.erase(
                            std::remove_if(
                                request_ids.begin(),
                                request_ids.end(),
                                [&](int id)
                                {
                                    return object_request_iters.find(id) == object_request_iters.end();
                                }),
                            request_ids.end());
                    }
                }
                disks[disk_id].head = (disks[disk_id].head + 1) % num_v;
            }
        }
    }
}