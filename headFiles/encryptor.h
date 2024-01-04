#include<cstdio> 
#include<cstring>
#include<cstdlib>
#include<sys/stat.h>
#include<sys/time.h>
#include<utime.h>
#include<iostream>
#include<fstream>
#include<time.h>
#include<unistd.h>
#include<string>
#include<fcntl.h>
#include<openssl/aes.h>

using namespace std;

class encryptor {
public:
    // 加密函数
    bool encrypt(string sourcefile, string targetfile, const unsigned char *key);
    // 解密函数
    bool decrypt(string sourcefile, string targetfile, const unsigned char *key);
};