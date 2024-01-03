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

#include "./headFiles/common.h"

using namespace std;

bool common::check_arg(string dir1, string dir2, int mode, string &msg) {
    if (mode < 0 || mode > 9) {
        msg = "invalid mode number";
        return false;
    }
    struct stat fileinfo;
    if (mode % 2 == 0) {  // 备份
        if (access(dir1.c_str(), F_OK) == -1) {
            msg = "folder to be backed up not exists";
            return false;
        }
        lstat(dir1.c_str(), &fileinfo);
        if (!S_ISDIR(fileinfo.st_mode)) {
            msg = "source path is not a folder";
            return false;
        }
        if (access(dir2.c_str(), F_OK) == -1) {
            msg = "the path stores backed up folder not exists";
            return false;
        }
        lstat(dir2.c_str(), &fileinfo);
        if (!S_ISDIR(fileinfo.st_mode)) {
            msg = "target path is not a folder";
            return false;
        }
    } else {  // 还原
        if (access(dir1.c_str(), F_OK) == -1) {
            msg = "the path stores restored folder not exists";
            return false;
        }
        lstat(dir1.c_str(), &fileinfo);
        if (!S_ISDIR(fileinfo.st_mode)) {
            msg = "the path stores restored folder is not a folder";
            return false;
        }
        if (access(dir2.c_str(), F_OK) == -1) {
            msg = "restored folder / file not exists";
            return false;
        }
        lstat(dir2.c_str(), &fileinfo);
        if (mode == 1) {
            if (!S_ISDIR(fileinfo.st_mode)) {
                msg = "folder to be restored path is not a folder";
                return false;
            }
        } else {
            if (!S_ISREG(fileinfo.st_mode)) {
                msg = "restored file is not a file";
                return false;
            }
            string suffix = dir2.substr(dir2.find_last_of(".") + 1, dir2.size() - dir2.find_last_of(".") - 1);
            if (mode == 3) {
                if (suffix != "bag") {
                    msg = "pack file type error";
                    return false;
                }
            } else if (mode == 5) {
                if (suffix != "comp") {
                    msg = "compress file type error";
                    return false;
                }
            } else if (mode == 7 || mode == 9) {
                if (suffix != "crypt") {
                    msg = "crypt file type error";
                    return false;
                }
            }
        }
    }
    return true;
}

string common::get_folder_name(string dir) {
    string filename, rel = "";
    for (int i = dir.size()-1; i > -1; i--) {
        if (dir[i] == '/') {
            filename = dir.substr(i, dir.size()-i);
            break;
        }
    }
    rel = rel + filename;
    return rel;
}

string common::change_suffix(string dir, string new_suffix) {
    dir.erase(dir.begin() + dir.find_last_of(".") + 1, dir.end());
    return dir + new_suffix;
}

void common::changeAttr(const char *src, const char *dst) {
    struct stat attr_of_src;
    lstat(src, &attr_of_src);

    //修改文件属性
    chmod(dst, attr_of_src.st_mode);
    //修改文件用户组
    chown(dst, attr_of_src.st_uid, attr_of_src.st_gid);

    //修改文件访问、修改时间
    if (S_ISLNK(attr_of_src.st_mode)) {
        struct timeval time_buf[2];
        time_buf[0].tv_sec = attr_of_src.st_atim.tv_sec;
        time_buf[0].tv_usec = attr_of_src.st_atim.tv_nsec / 1000;
        time_buf[1].tv_sec = attr_of_src.st_mtim.tv_sec;
        time_buf[1].tv_usec = attr_of_src.st_mtim.tv_nsec / 1000;
        if (lutimes(dst, time_buf) == -1) {
            printf("%s\n", dst);
            perror("lutimes");
        }
    } else {
        struct utimbuf tbuf;
        tbuf.actime = attr_of_src.st_atime;
        tbuf.modtime = attr_of_src.st_mtime;
        utime(dst, &tbuf);
    }
}

// 定义一个函数，用于复制一个文件
void common::copy_file(const char* source_file, const char* target_file) {
    std::string source_filename = source_file;
    std::string target_filename = target_file;

    std::ifstream fin(source_filename, std::ios::in);
    std::ofstream fout(target_filename, std::ios::out);

    if (fin && fout) {
        std::string line;
        while (getline(fin, line)) {
            fout << line << std::endl;
        }
    }

    //保留元数据
    changeAttr(source_file, target_file);

    fin.close();
    fout.close();
}

void common::copySln(const char *src_file, const char *dst_file) {
    //软链接所指向的文件路径
    char slink_path[1024];
    memset(slink_path, 0, sizeof(slink_path));
    int len = 0;
    if ((len = readlink(src_file, slink_path, sizeof(slink_path))) > 0) {
        char src_dir[1024];
        strcpy(src_dir, src_file);
        //获取源文件的绝对路径，不含文件名
        for (int i = strlen(src_dir); i > -1; i--) {
            if (src_dir[i] == '/') {
                src_dir[i + 1] = 0;
                break;
            }
        }
        if (slink_path[0] != '/') //一定是相对路径，如果首字符是'/'则是从根目录出发的绝对路径
            strcat(src_dir, slink_path); //将相对软链接的路径添加到软链接文件路径的后面
        else
            strcpy(src_dir, slink_path); //绝对路径拷贝过去即可
        //保存软链接所指向的文件的名字
        char file_name[1024];
        //提取所指向的文件的目录，由于可能是相对路径，故切换到对应的文件夹下获取绝对路径
        for (int i = strlen(src_dir); i > -1; i--) {
            if (src_dir[i] == '/') {
                strcpy(file_name, src_dir + i); // 将软链接所指向的文件名保存在file_name中
                src_dir[i + 1] = 0;
                break;
            }
        }
        char *cur_path = getcwd(NULL, 0);
        chdir(src_dir);  //切换到软链接所指向的文件的路径
        strcpy(src_dir, getcwd(NULL, 0));  //获取软链接所指向文件的绝对路径
        strcat(src_dir, file_name);  //添加软链接所指向的文件名即组成了绝对路径
        chdir(cur_path);
        //创建软链接
        if (symlink(src_dir, dst_file) == -1) {
            perror("symlink");
        }
        //修改目标软链接属性使其与源软链接属性一致
        changeAttr(src_file, dst_file);
    } else {
        printf("%s: 软链接错误\n", src_file);
    }
}

void common::copyPipe(const char *src_file, const char *dst_file) {
    struct stat attr_of_src;
    stat(src_file, &attr_of_src);
    mkfifo(dst_file, attr_of_src.st_mode);
    changeAttr(src_file, dst_file);
}

// 定义一个函数，用于复制一个目录及其内容
void common::copy_directory(string source_dirname, string target_dirname, vector<string> filter_arg) {
    DIR *source_dir = opendir(source_dirname.c_str());
    if (source_dir == NULL) {
        printf("cannot open source directory\n");
        exit(1);
    }
    // 创建目标文件夹
    int result = mkdir(target_dirname.c_str(), 0777);
    if (result == -1) {
        printf("cannot create target directory\n");
        exit(1);
    }

    // 遍历源文件夹下的所有文件和子文件夹
    struct dirent *entry;
    while ((entry = readdir(source_dir)) != NULL) {
        struct stat fileinfo;
        // 跳过.和..两个特殊的目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 获取该文件的路径
        string file_path = source_dirname + "/" + entry->d_name;
        if (lstat(file_path.c_str(), &fileinfo) != 0) {
            cout << file_path << endl;
            printf("cannot create stat struct\n");
            exit(1);
        }

        // 获取该文件的类型,修改时间,文件大小属性
        bool match;
        if (entry->d_type != DT_DIR) {
            match = filter::filejudge(filter_arg, entry->d_name, file_path, fileinfo);
        }

        // 拼接源和目标的完整路径
        std::string source_path = source_dirname + "/" + entry->d_name;
        std::string target_path = target_dirname + "/" + entry->d_name;

        // 判断是什么类型,分别对应处理
        if (entry->d_type == DT_DIR) {  // 子文件夹
            copy_directory(source_path, target_path, filter_arg);  // 递归调用复制文件夹的函数
        } else if (entry->d_type == DT_LNK) {  // 软链接
            if (match) copySln(source_path.c_str(), target_path.c_str());
        } else if (entry->d_type == DT_FIFO) {  // 管道文件
            if (match) copyPipe(source_path.c_str(), target_path.c_str());
        } else if (entry->d_type == DT_REG) {  // 普通文件
            if (match) copy_file(source_path.c_str(), target_path.c_str());  // 调用复制文件的函数
        }
    }

    // 关闭源文件夹
    closedir(source_dir);
}
