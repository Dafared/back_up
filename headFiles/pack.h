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

#include"./filter.h"

using namespace std;

#define BLOCKSIZE 512
typedef struct
{
    char name[100];
    char mode[32];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime_sec[12];
    char mtime_nsec[12];
    char typeflag;
    char linkname[100];
    char fileflag;
    char padding[218];
} headblock; // 块结构定义

class pack_class{
public:
    pack_class() = default;
    ~pack_class() = default;
    // 获取文件夹大小
    long long int sizeofDir(string dir_path);
    // 生成head block
    headblock* getHeadBlock(string file_path, string relative);
    // 打包接口
    void pack(string source_dirname, int target_file, vector<string> filter_arg);
    // 打包函数
    bool packToFile(string source_dirname, int target_file, vector<string> filter_arg, string rel);
    // 解包函数
    bool unpack(int source_file, string target_dirname);
};