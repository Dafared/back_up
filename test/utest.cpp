#include "../headFiles/common.h"
#include "../headFiles/pack.h"
#include"../headFiles/compress.h"
#include"../headFiles/encryptor.h"
#include "../headFiles/filter.h"

using namespace std;

int main(int argc, char** argv) {
    // 检查参数个数 
    if (argc < 4) { 
        cout<<"argument error"<<endl;
        exit(1); 
    }

    pack_class pack;
    compress_class comp;
    common com;
    encryptor crypt;
    filter fil;

    /**
     * 备份:
     * source_dir(argv[1]):要测试的备份源文件/文件夹路径
     * target_dir(argv[2]):要储存该文件/文件夹备份的地址
     * 还原:
     * source_dir(argv[1]):要测试还原该文件/文件夹备份的地址
     * target_dir(argv[2]):文件夹备份版本/打包文件/加密文件/压缩文件的地址
     * 测试过滤器:
     * source_dir(argv[1]):要用于测试的文件
     * target_dir(argv[2]):占位符0
    */
    const char* source_dir = argv[1];
    const char* target_dir = argv[2];

    // 单元测试代码:0:复制普通文件 1:复制软链接文件 2.复制管道文件 3.测试自定义条件过滤器 4.正常备份文件夹
    // 5.正常还原文件夹 6.打包文件 7.解包文件 8.压缩文件 9.解压文件 10.加密文件 11.解密文件
    int mode = stoi(string(argv[3]));
    
    // 自定义备份的文件筛选条件(5个参数,argv[4]-argv[8])
    // 分别为:类型,大小(单位:字节,数字前面: s表示<=; b表示>=),名称,修改时间,路径(条件缺省占位符:0)
    // 输入时间字符串形如: b2021-12-01 12:00:00(s:早于;b:晚于)
    // 指定要筛选的类型:软链接文件:"link";管道文件:"pipe";普通文件:匹配文件名中'.'后面的内容
    // 要筛选多种类型,中间用'/'分隔,结尾也要有'/'(示例:link/txt/pdf/jpg/zip/)
    vector<string> filter_arg = {"0", "0", "0", "0", "0"};
    if(mode == 3) {
        if (argc != 9) {
            cout<<"argument error"<<endl;
            exit(1); 
        }
        if (!fil.checkfilter(argv)) {
            cout<<"filter arguments format error"<<endl;
            exit(1);
        }
        int j = 0;
        for(int i=4;i<=8;i++){
            filter_arg[j++] = (argv[i]);
        }
    }

    string input_key;
    unsigned char *key;
    // 如果要求加密,用户输入加密使用的密码
    if (mode >= 10) {
        cout<<"please input password:"<<endl;     
        while (true) {
            cin>>input_key;
            if(input_key.size() == 16) {
                break;
            }
            cout<<"invalid password length:"<<endl; 
        }
        key = reinterpret_cast<unsigned char *>(const_cast<char *>(input_key.data()));
    }

    switch (mode) {
        case 0 : // 复制普通文件
        {   
            com.copy_file(source_dir,target_dir);
            break;
        }
        case 1 : // 复制软链接文件
        {
            com.copySln(source_dir,target_dir);
            break;
        }
        case 2 : // 复制管道文件
        {
            com.copyPipe(source_dir,target_dir);
            break;
        }
        case 3 : // 测试自定义条件过滤器
        {
            filter::testfilter(source_dir,filter_arg);
            break;
        }
        case 4 : // 正常备份文件夹
        {
            string target_folder_path = string(target_dir) + com.get_folder_name(source_dir);
            com.copy_directory(source_dir, target_folder_path, filter_arg);
            break;
        }
        case 5 : // 正常还原文件夹
        {   
            string recover_file_path = string(source_dir) + com.get_folder_name(target_dir);
            com.copy_directory(target_dir, recover_file_path, {"0", "0", "0", "0", "0"});
            break;
        }
        case 6 : // 打包文件夹
        {
            string target_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".bag";
            int fout = open(target_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
            pack.pack(source_dir, fout, filter_arg);
            break;
        }
        case 7 : // 解包文件
        {
            int fin = open(target_dir, O_RDONLY);
            pack.unpack(fin,source_dir);
            break;
        }
        case 8 : // 压缩文件
        {
            string target_comp_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".comp";
            comp.compress(source_dir, target_comp_file_path);
            break;
        }
        case 9 : // 解压文件
        {   
            string filename = com.change_suffix(com.get_folder_name(target_dir),"");
            string decomp_path = string(source_dir) + filename.substr(0,filename.size()-1);
            comp.decompress(target_dir, decomp_path);
            break;
        }
        case 10 : // 加密文件
        {
            string target_crypt_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".crypt";
            crypt.encrypt(source_dir, target_crypt_file_path, key);
            break;
        }
        case 11 : // 解密文件
        {   
            string filename = com.change_suffix(com.get_folder_name(target_dir),"");
            string decrypt_path = string(source_dir) + filename.substr(0,filename.size()-1);
            crypt.decrypt(target_dir, decrypt_path, key);
            break;
        }
    }

    return 0;
}
