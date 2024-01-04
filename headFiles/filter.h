#pragma once
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
#include <regex>
#include <ctime>
#include <chrono>

using namespace std;

class filter{
public:
    filter() = default;
    ~filter() = default;
    // 判断自定义备份文件大小条件
    static bool sizejudge(off_t byte_size, string match_size);
    // 判断自定义备份文件类型条件
    static bool typejudge(string type, string types);
    // 判断自定义备份修改时间条件
    static bool timejudge(time_t time, string match_time);
    // 判断自定义备份文件名条件
    static bool namejudge(string name, string match_name);
    // 判断自定义备份所属路径条件
    static bool pathjudge(string path, string match_path);
    // 判断文件是否符合filter_arg给定条件
    static bool filejudge(vector<string> filter_arg, string type, off_t byte_size, string name, time_t time, string path);
    static bool filejudge(vector<string> filter_arg, string name, string path, struct stat info);
    // 用于检查用户输入的自定义备份条件格式是否正确
    static bool checkfilter(char** argv);
    // 检查输入的日期时间格式是否正确
    static bool checkdatetime(string str);
    // 用于单元测试自定义条件过滤器的函数
    static void testfilter(string test_file, vector<string> filter_arg);
};