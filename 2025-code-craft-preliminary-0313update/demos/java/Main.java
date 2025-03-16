import java.util.Scanner;

public class Main {
    static int T, M, N, V, G;

    static final int FRE_PER_SLICING = 1800;

    static final int MAX_DISK_NUM = (10 + 1);

    static final int MAX_DISK_SIZE = (16384 + 1);

    static final int MAX_REQUEST_NUM = (30000000 + 1);

    static final int MAX_OBJECT_NUM = (100000 + 1);

    static final int REP_NUM = (3);

    static final int EXTRA_TIME = (105);

    static int[][] disk = new int[MAX_DISK_NUM][MAX_DISK_SIZE];

    static int[] diskPoint = new int[MAX_DISK_NUM];

    static int[] objectIds = new int[MAX_OBJECT_NUM];

    static Scanner scanner = new Scanner(System.in);

    static Request[] requests = new Request[MAX_REQUEST_NUM];

    static Object[] objects = new Object[MAX_OBJECT_NUM];

    static int currentRequest = 0;

    static int currentPhase = 0;

    static class Request {
        int objectId;

        int prevId;

        boolean isDone;
    }

    static class Object {
        int[] replica = new int[REP_NUM + 1];

        int[][] unit = new int[REP_NUM + 1][];

        int size;

        int lastRequestPoint;

        boolean isDelete;
    }

    static {
        for (int i = 0; i < MAX_REQUEST_NUM; i++) {
            requests[i] = new Request();
        }
        for (int i = 0; i < MAX_OBJECT_NUM; i++) {
            objects[i] = new Object();
        }
    }

    public static void main(String[] args) {
        T = scanner.nextInt();
        M = scanner.nextInt();
        N = scanner.nextInt();
        V = scanner.nextInt();
        G = scanner.nextInt();
        skipPreprocessing();
        skipPreprocessing();
        skipPreprocessing();
        System.out.println("OK");
        System.out.flush();
        for (int i = 1; i <= N; i++) {
            diskPoint[i] = 1;
        }
        for (int t = 1; t <= T + EXTRA_TIME; t++) {
            timestampAction();
            deleteAction();
            writeAction();
            readAction();
        }
        scanner.close();
    }

    private static void skipPreprocessing() {
        for (int i = 1; i <= M; i++) {
            for (int j = 1; j <= (T - 1) / FRE_PER_SLICING + 1; j++) {
                scanner.nextInt();
            }
        }
    }

    private static void timestampAction() {
        scanner.next();
        int timestamp = scanner.nextInt();
        System.out.printf("TIMESTAMP %d\n", timestamp);
        System.out.flush();
    }

    private static void deleteAction() {
        int nDelete = scanner.nextInt();
        int abortNum = 0;
        for (int i = 1; i <= nDelete; i++) {
            objectIds[i] = scanner.nextInt();
        }
        for (int i = 1; i <= nDelete; i++) {
            int id = objectIds[i];
            int currentId = objects[id].lastRequestPoint;
            while (currentId != 0) {
                if (!requests[currentId].isDone) {
                    abortNum++;
                }
                currentId = requests[currentId].prevId;
            }
        }
        System.out.printf("%d\n", abortNum);
        for (int i = 1; i <= nDelete; i++) {
            int id = objectIds[i];
            int currentId = objects[id].lastRequestPoint;
            while (currentId != 0) {
                if (!requests[currentId].isDone) {
                    System.out.printf("%d\n", currentId);
                }
                currentId = requests[currentId].prevId;
            }
            for (int j = 1; j <= REP_NUM; j++) {
                doObjectDelete(objects[id].unit[j], disk[objects[id].replica[j]], objects[id].size);
            }
            objects[id].isDelete = true;
        }
        System.out.flush();
    }

    private static void doObjectDelete(int[] objectUnit, int[] diskUnit, int size) {
        for (int i = 1; i <= size; i++) {
            diskUnit[objectUnit[i]] = 0;
        }
    }

    private static void writeAction() {
        int nWrite = scanner.nextInt();
        for (int i = 1; i <= nWrite; i++) {
            int id, size;
            id = scanner.nextInt();
            size = scanner.nextInt();
            scanner.nextInt();
            objects[id].lastRequestPoint = 0;
            for (int j = 1; j <= REP_NUM; j++) {
                objects[id].replica[j] = (id + j) % N + 1;
                objects[id].unit[j] = new int[size + 1];
                objects[id].size = size;
                objects[id].isDelete = false;
                doObjectWrite(objects[id].unit[j], disk[objects[id].replica[j]], size, id);
            }

            System.out.printf("%d\n", id);
            for (int j = 1; j <= REP_NUM; j++) {
                System.out.printf("%d", objects[id].replica[j]);
                for (int k = 1; k <= size; k++) {
                    System.out.printf(" %d", objects[id].unit[j][k]);
                }
                System.out.println();
            }
        }
        System.out.flush();
    }

    private static void doObjectWrite(int[] objectUnit, int[] diskUnit, int size, int objectId) {
        int currentWritePoint = 0;
        for (int i = 1; i <= V; i++) {
            if (diskUnit[i] == 0) {
                diskUnit[i] = objectId;
                objectUnit[++currentWritePoint] = i;
                if (currentWritePoint == size) {
                    break;
                }
            }
        }
    }

    private static void readAction() {
        int nRead;
        int requestId = 0, objectId;
        nRead = scanner.nextInt();
        for (int i = 1; i <= nRead; i++) {
            requestId = scanner.nextInt();
            objectId = scanner.nextInt();
            requests[requestId].objectId = objectId;
            requests[requestId].prevId = objects[objectId].lastRequestPoint;
            objects[objectId].lastRequestPoint = requestId;
            requests[requestId].isDone = false;
        }
        if (currentRequest == 0 && nRead > 0) {
            currentRequest = requestId;
        }
        processCurrentReadRequest();
    }

    private static void processCurrentReadRequest() {
        int objectId;
        if (currentRequest == 0) {
            for (int i = 1; i <= N; i++) {
                System.out.println("#");
            }
            System.out.println("0");
        } else {
            currentPhase++;
            objectId = requests[currentRequest].objectId;
            for (int i = 1; i <= N; i++) {
                if (i == objects[objectId].replica[1]) {
                    if (currentPhase % 2 == 1) {
                        System.out.printf("j %d\n", objects[objectId].unit[1][currentPhase / 2 + 1]);
                    } else {
                        System.out.println("r#");
                    }
                } else {
                    System.out.println("#");
                }
            }

            if (currentPhase == objects[objectId].size * 2) {
                if (objects[objectId].isDelete) {
                    System.out.println("0");
                } else {
                    System.out.printf("1\n%d\n", currentRequest);
                    requests[currentRequest].isDone = true;
                }
                currentRequest = 0;
                currentPhase = 0;
            } else {
                System.out.println("0");
            }
        }
        System.out.flush();
    }
}