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
    bool encrypt(string sourcefile, string targetfile, const unsigned char *key);
    bool decrypt(string sourcefile, string targetfile, const unsigned char *key);
};