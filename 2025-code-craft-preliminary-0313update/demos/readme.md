### 文件清单
```
.
├── readme.md -------------------------- 说明文件
├── c ---------------------------------- C语言工程，CMakeLists.txt文件必选，内容部分可自定义
|   ├── CMakeLists.txt
|   └── main.c
├── c++ -------------------------------- C++语言工程，CMakeLists.txt文件必选，内容部分可自定义
|   ├── CMakeLists.txt
|   └── main.cpp
├── java ------------------------------- Java语言工程，main.java文件必选
|   └── main.java
└── python ----------------------------- Python语言工程，main.py文件必选
    └── main.py
```

### 本地编译与调试
```
C、C++的编译：
进入工程目录，运行
cmake .
make

Java的编译：
javac main.java

```