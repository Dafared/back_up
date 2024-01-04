#include"./headFiles/encryptor.h"

// 加密函数
bool encryptor::encrypt(string sourcefile, string targetfile, const unsigned char *key) {
    unsigned char buffer_in[AES_BLOCK_SIZE];
    unsigned char buffer_out[AES_BLOCK_SIZE];
    AES_KEY encrypt_key;
    AES_set_encrypt_key(key, 128, &encrypt_key);

    struct stat fileinfo;
    if(lstat(sourcefile.c_str(),&fileinfo)!=0){
        printf("cannot create stat struct\n");
        exit(1);
    }
    if (!S_ISREG(fileinfo.st_mode)) {
        cout<<"invalid input file!";
        return false;
    }

    int fin = open(sourcefile.c_str(), O_RDONLY);
    int fout = open(targetfile.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);
    int reminder = fileinfo.st_size % AES_BLOCK_SIZE;

    if (fileinfo.st_size <= AES_BLOCK_SIZE) {
        read(fin, buffer_in, fileinfo.st_size);
        // 若文件字节数小于AES_BLOCK_SIZE,需要填充到AES_BLOCK_SIZE(填充内容为填充长度值)
        for(size_t i = fileinfo.st_size; i<AES_BLOCK_SIZE; i++) {
            buffer_in[i] = reminder;
        }
        AES_ecb_encrypt(buffer_in, buffer_out, &encrypt_key, AES_ENCRYPT);
        write(fout, buffer_out, AES_BLOCK_SIZE);
    } else {
        // 加密除倒数第一个块外其他块
        for (int i = 0; i < (fileinfo.st_size / AES_BLOCK_SIZE); i++) {
            read(fin, buffer_in, AES_BLOCK_SIZE);
            AES_ecb_encrypt(buffer_in, buffer_out, &encrypt_key, AES_ENCRYPT);
            write(fout, buffer_out, AES_BLOCK_SIZE);
        }
        // 若文件最后剩余字节数小于AES_BLOCK_SIZE,需要填充到AES_BLOCK_SIZE(填充内容为填充长度值)
        if (reminder != 0) {
            read(fin, buffer_in, reminder);
            for(size_t i = reminder; i<AES_BLOCK_SIZE; i++) {
                buffer_in[i] = AES_BLOCK_SIZE - reminder;
            }
            AES_ecb_encrypt(buffer_in, buffer_out, &encrypt_key, AES_ENCRYPT);
            write(fout, buffer_out, AES_BLOCK_SIZE);
        }    
    }
    // 若文件长度可正好被AES_BLOCK_SIZE整除,则另外补一个块,内容为AES_BLOCK_SIZE,用以表示无需填充
    if (reminder == 0) {
        for (int i = 0; i < AES_BLOCK_SIZE; i++) {
            buffer_in[i] = AES_BLOCK_SIZE;
        }
        AES_ecb_encrypt(buffer_in, buffer_out, &encrypt_key, AES_ENCRYPT);
        write(fout, buffer_out, AES_BLOCK_SIZE);
    }

    close(fin);
    close(fout);
    return true;
}

// 解密函数
bool encryptor::decrypt(string sourcefile, string targetfile, const unsigned char *key) {
    unsigned char buffer_in[AES_BLOCK_SIZE];
    unsigned char buffer_out[AES_BLOCK_SIZE];
    AES_KEY decrypt_key;
    AES_set_decrypt_key(key, 128, &decrypt_key);

    struct stat fileinfo;
    if(lstat(sourcefile.c_str(), &fileinfo)!=0){
        printf("cannot create stat struct\n");
        exit(1);
    }
    if (!S_ISREG(fileinfo.st_mode)) {
        cout<<"invalid input file!";
        return false;
    }

    int fin = open(sourcefile.c_str(), O_RDONLY);
    int fout = open(targetfile.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644);

    // 解密除最后一个块外其他块
    for (int i = 0; i < (fileinfo.st_size / AES_BLOCK_SIZE) - 1; i++) {
        read(fin,buffer_in,AES_BLOCK_SIZE);
        AES_ecb_encrypt(buffer_in, buffer_out, &decrypt_key, AES_DECRYPT);
        write(fout,buffer_out,AES_BLOCK_SIZE);
    }
    // 解密最后一个块
    read(fin,buffer_in,AES_BLOCK_SIZE);
    AES_ecb_encrypt(buffer_in, buffer_out, &decrypt_key, AES_DECRYPT);
    int padding_size = buffer_out[AES_BLOCK_SIZE-1];
    if (padding_size != AES_BLOCK_SIZE) {
        for (int i = 0; i < AES_BLOCK_SIZE - padding_size; i++) {
            write(fout,&buffer_out[i],1);
        }
    }

    close(fin);
    close(fout);
    return true;
}