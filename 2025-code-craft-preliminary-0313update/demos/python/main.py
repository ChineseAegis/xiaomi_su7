import sys

FRE_PER_SLICING = 1800
MAX_DISK_NUM = (10 + 1)
MAX_DISK_SIZE = (16384 + 1)
MAX_REQUEST_NUM = (30000000 + 1)
MAX_OBJECT_NUM = (100000 + 1)
REP_NUM = 3
EXTRA_TIME = 105

disk = [[0 for _ in range(MAX_DISK_SIZE)] for _ in range(MAX_DISK_NUM)]
disk_point = [0 for _ in range(MAX_DISK_NUM)]
_id = [0 for _ in range(MAX_OBJECT_NUM)]

current_request = 0
current_phase = 0


class Object:
    def __init__(self):
        self.replica = [0 for _ in range(REP_NUM + 1)]
        self.unit = [[] for _ in range(REP_NUM + 1)]
        self.size = 0
        self.lastRequestPoint = 0
        self.isDelete = False


req_object_ids = [0] * MAX_REQUEST_NUM
req_prev_ids = [0] * MAX_REQUEST_NUM
req_is_dones = [False] * MAX_REQUEST_NUM

objects = [Object() for _ in range(MAX_OBJECT_NUM)]


def do_object_delete(object_unit, disk_unit, size):
    for i in range(1, size + 1):
        disk_unit[object_unit[i]] = 0


def timestamp_action():
    timestamp = input().split()[1]
    print(f"TIMESTAMP {timestamp}")
    sys.stdout.flush()


def delete_action():
    n_delete = int(input())
    abortNum = 0
    for i in range(1, n_delete + 1):
        _id[i] = int(input())
    for i in range(1, n_delete + 1):
        delete_id = _id[i]
        currentId = objects[delete_id].lastRequestPoint
        while currentId != 0:
            if not req_is_dones[currentId]:
                abortNum += 1
            currentId = req_prev_ids[currentId]

    print(f"{abortNum}")
    for i in range(n_delete + 1):
        delete_id = _id[i]
        currentId = objects[delete_id].lastRequestPoint
        while currentId != 0:
            if not req_is_dones[currentId]:
                print(f"{currentId}")
            currentId = req_prev_ids[currentId]
        for j in range(1, REP_NUM + 1):
            do_object_delete(objects[delete_id].unit[j], disk[objects[delete_id].replica[j]], objects[delete_id].size)
        objects[delete_id].isDelete = True
    sys.stdout.flush()


def do_object_write(object_unit, disk_unit, size, object_id):
    current_write_point = 0
    for i in range(1, V + 1):
        if disk_unit[i] == 0:
            disk_unit[i] = object_id
            current_write_point += 1
            object_unit[current_write_point] = i
            if current_write_point == size:
                break
    assert (current_write_point == size)


def write_action():
    n_write = int(input())
    for i in range(1, n_write + 1):
        write_input = input().split()
        write_id = int(write_input[0])
        size = int(write_input[1])
        objects[write_id].lastRequestPoint = 0
        for j in range(1, REP_NUM + 1):
            objects[write_id].replica[j] = (write_id + j) % N + 1
            objects[write_id].unit[j] = [0 for _ in range(size + 1)]
            objects[write_id].size = size
            objects[write_id].isDelete = False
            do_object_write(objects[write_id].unit[j], disk[objects[write_id].replica[j]], size, write_id)
        print(f"{write_id}")
        for j in range(1, REP_NUM + 1):
            print_next(f"{objects[write_id].replica[j]}")
            for k in range(1, size + 1):
                print_next(f" {objects[write_id].unit[j][k]}")
            print()
    sys.stdout.flush()


def read_action():
    request_id = 0
    nRead = int(input())
    for i in range(1, nRead + 1):
        read_input = input().split()
        request_id = int(read_input[0])
        objectId = int(read_input[1])
        req_object_ids[request_id] = objectId
        req_prev_ids[request_id] = objects[objectId].lastRequestPoint
        objects[objectId].lastRequestPoint = request_id
        req_is_dones[request_id] = False
    global current_request
    global current_phase
    if current_request == 0 and nRead > 0:
        current_request = request_id
    if current_request == 0:
        for i in range(1, N + 1):
            print("#")
        print("0")
    else:
        current_phase += 1
        objectId = req_object_ids[current_request]
        for i in range(1, N + 1):
            if i == objects[objectId].replica[1]:
                if current_phase % 2 == 1:
                    print(f"j {objects[objectId].unit[1][int(current_phase / 2 + 1)]}")
                else:
                    print("r#")
            else:
                print("#")
        if current_phase == objects[objectId].size * 2:
            if objects[objectId].isDelete:
                print("0")
            else:
                print(f"1\n{current_request}")
                req_is_dones[current_request] = True
            current_request = 0
            current_phase = 0
        else:
            print("0")
    sys.stdout.flush()


def print_next(message):
    print(f"{message}", end="")


if __name__ == '__main__':
    user_input = input().split()
    T = int(user_input[0])
    M = int(user_input[1])
    N = int(user_input[2])
    V = int(user_input[3])
    G = int(user_input[4])
    # skip preprocessing
    for item in range(1, M * 3 + 1):
        input()
    print("OK")
    sys.stdout.flush()
    for item in range(1, N + 1):
        disk_point[item] = 1
    for item in range(1, T + EXTRA_TIME + 1):
        timestamp_action()
        delete_action()
        write_action()
        read_action()
