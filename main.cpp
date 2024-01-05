#include "./headFiles/common.h"
#include "./headFiles/pack.h"
#include"./headFiles/compress.h"
#include"./headFiles/encryptor.h"
#include "./headFiles/filter.h"

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
     * source_dir(argv[1]):要备份的文件夹路径
     * target_dir(argv[2]):要储存该文件夹备份的地址
     * 还原:
     * source_dir(argv[1]):要还原该文件夹的地址
     * target_dir(argv[2]):该文件夹备份版本/打包文件/加密文件/压缩文件的地址
    */
    const char* source_dir = argv[1];
    const char* target_dir = argv[2];

    // 模式代码:0: 正常备份 1: 正常还原 2: 打包备份 3: 解包还原 4:打包+压缩
    // 5:解压+解包 6.打包+加密 7.解密+解包 8.打包+压缩+加密 9.解密+解压+解包
    int mode = stoi(string(argv[3]));

    // 检查输入路径的有效性(包括路径/文件是否存在,以及文件路径是否符合要求),以及模式代码的有效性
    string msg;
    if (!com.check_arg(source_dir, target_dir, mode, msg)) {
        cout<<msg<<endl;
        exit(1);
    }

    // 自定义备份的文件筛选条件(5个参数,argv[4]-argv[8])
    // 分别为:类型,大小(单位:字节,数字前面: s表示<=; b表示>=),名称,修改时间,路径(条件缺省占位符:0)
    // 输入时间字符串形如: b2021-12-01 12:00:00(s:早于;b:晚于)
    // 指定要筛选的类型:软链接文件:"link";管道文件:"pipe";普通文件:匹配文件名中'.'后面的内容
    // 要筛选多种类型,中间用'/'分隔,结尾也要有'/'(示例:link/txt/pdf/jpg/zip/)
    vector<string> filter_arg = {"0", "0", "0", "0", "0"};
    if (argc == 9) {
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
    if (mode >= 6) {
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
    
    if(mode == 0){ // 正常备份文件夹(source_dir)至target_dir位置
        string target_folder_path = string(target_dir) + com.get_folder_name(source_dir);
        com.copy_directory(source_dir, target_folder_path, filter_arg);
    }
    else if(mode == 1){ // 把文件夹target_dir还原到source_dir位置
        string recover_file_path = string(source_dir) + com.get_folder_name(target_dir);
        com.copy_directory(target_dir, recover_file_path, {"0", "0", "0", "0", "0"});
    }
    else if(mode == 2){ // 把文件夹(source_dir)打包并备份到target_dir位置
        string target_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".bag";
        int fout = open(target_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
        pack.pack(source_dir, fout, filter_arg);
    }   
    else if(mode == 3){ // 把路径为target_dir的打包文件解包并将其中内容还原到source_dir位置
        int fin = open(target_dir, O_RDONLY);
        pack.unpack(fin,source_dir);
    }
    else if(mode == 4){ // 把文件夹(source_dir)打包+压缩并备份到target_dir位置
        string target_pack_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".bag";
        string target_comp_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".comp";
        int pack_fout = open(target_pack_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);

        pack.pack(source_dir, pack_fout, filter_arg);
        comp.compress(target_pack_file_path, target_comp_file_path);
    }
    else if(mode == 5){ // 把路径为target_dir的压缩文件解压+解包并将其中内容还原到source_dir位置
        string target_pack_file_path = com.change_suffix(target_dir,"bag");
        comp.decompress(target_dir, target_pack_file_path);
        int fin = open(target_pack_file_path.c_str(), O_RDONLY);
        pack.unpack(fin,source_dir);
    }
    else if(mode == 6){ // 把文件夹(source_dir)打包+加密并备份到target_dir位置
        string target_pack_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".bag";
        string target_crypt_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".crypt";
        int pack_fout = open(target_pack_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);

        pack.pack(source_dir, pack_fout, filter_arg);
        crypt.encrypt(target_pack_file_path, target_crypt_file_path, key);
    }
    else if(mode == 7){ // 把路径为target_dir的压缩文件解密+解包并将其中内容还原到source_dir位置
        string target_pack_file_path = com.change_suffix(target_dir,"bag");
        crypt.decrypt(target_dir, target_pack_file_path, key);
        int fin = open(target_pack_file_path.c_str(), O_RDONLY);
        pack.unpack(fin,source_dir);
    }
    else if(mode == 8){ // 把文件夹(source_dir)打包+压缩+加密并备份到target_dir位置
        string target_pack_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".bag";
        string target_comp_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".comp";
        string target_crypt_file_path = string(target_dir) + com.get_folder_name(source_dir) + ".crypt";
        int pack_fout = open(target_pack_file_path.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);

        pack.pack(source_dir, pack_fout, filter_arg);
        comp.compress(target_pack_file_path, target_comp_file_path);
        crypt.encrypt(target_comp_file_path, target_crypt_file_path, key);
    }
    else if(mode == 9){ // 把路径为target_dir的压缩文件解密+解压+解包并将其中内容还原到source_dir位置
        string target_pack_file_path = com.change_suffix(target_dir,"bag");
        string target_comp_file_path = com.change_suffix(target_dir,"comp");

        crypt.decrypt(target_dir, target_comp_file_path, key);
        comp.decompress(target_comp_file_path, target_pack_file_path);
        int fin = open(target_pack_file_path.c_str(), O_RDONLY);
        pack.unpack(fin,source_dir);
    }

    return 0; 
}