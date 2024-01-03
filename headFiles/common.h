#include<cstdio> 
#include<cstring>
#include<cstdlib>
#include<dirent.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<utime.h>
#include<iostream>
#include<fstream>
#include<time.h>
#include<unistd.h>
#include<unordered_set>
#include<vector>
#include<string>
#include<fcntl.h>

#include "./filter.h"
using namespace std;

class common{
public:
    common() = default;
    ~common() = default;

    // 复制一个文件
    void copy_file(const char* source_file, const char* target_file);

    // 复制文件元数据
    void changeAttr(const char *src, const char *dst);

    // 拷贝软链接
    void copySln(const char *src_file, const char *dst_file);

    // 拷贝管道
    void copyPipe(const char *src_file, const char *dst_file);

    // 拷贝一个目录
    void copy_directory(string source_dirname, string target_dirname, vector<string> filter_arg);

    // 获取路径末尾文件夹名称
    string get_folder_name(string dir);

    // 改变文件路径中文件类型后缀
    string change_suffix(string dir, string new_suffix);

    // 检查输入路径的有效性(包括路径/文件是否存在,以及文件路径是否符合要求),以及模式代码的有效性
    bool check_arg(string dir1, string dir2, int mode, string &msg);
};