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
    static bool sizejudge(off_t byte_size, string match_size);
    static bool typejudge(string type, string types);
    static bool timejudge(time_t time, string match_time);
    static bool namejudge(string name, string match_name);
    static bool pathjudge(string path, string match_path);
    static bool filejudge(vector<string> filter_arg, string type, off_t byte_size, string name, time_t time, string path);
    static bool filejudge(vector<string> filter_arg, string name, string path, struct stat info);
    static bool checkfilter(char** argv);
    static bool checkdatetime(string str);
    static void testfilter(string test_file, vector<string> filter_arg);
};