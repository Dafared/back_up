#include "./headFiles/pack.h"

using namespace std;

// 获取文件夹大小
long long int pack_class::sizeofDir(string dir_path){
    DIR *dir;
	struct dirent *entry;
	struct stat fileinfo;
    long long int totalDirSize = 0;
    if ((dir = opendir(dir_path.c_str())) == NULL){
		perror("dir");
	}
    lstat(dir_path.c_str(), &fileinfo);
    totalDirSize += fileinfo.st_size;
    while((entry = readdir(dir))!=NULL){
        // 跳过.和..两个特殊的目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 获取该文件的路径
        string subdir_path = dir_path + "/" + entry->d_name;
        if(lstat(subdir_path.c_str(),&fileinfo)!=0){
            printf("cannot create stat struct\n");
            exit(1);
        }

        if(S_ISDIR(fileinfo.st_mode)){
            totalDirSize += sizeofDir(subdir_path);
        }else{
            totalDirSize += fileinfo.st_size;
        }
    }
    closedir(dir);
    return totalDirSize;
}

// 生成head block
headblock* pack_class::getHeadBlock(string file_path, string relative){
    struct stat fileinfo;
    lstat(file_path.c_str(), &fileinfo);
    headblock *head = new headblock();
    strcpy(head->name, relative.c_str());
    strcpy(head->mode, (to_string(fileinfo.st_mode)).c_str());
    strcpy(head->uid,(to_string(fileinfo.st_uid)).c_str());
    strcpy(head->gid,(to_string(fileinfo.st_gid)).c_str());   
    if(S_ISDIR(fileinfo.st_mode)){
        strcpy(head->size,(to_string(sizeofDir(file_path))).c_str());
    }else{
        strcpy(head->size,(to_string(fileinfo.st_size)).c_str());
    }
    strcpy(head->mtime_sec,(to_string(fileinfo.st_mtim.tv_sec)).c_str());
    strcpy(head->mtime_nsec,(to_string(fileinfo.st_mtim.tv_nsec)).c_str());
    if (S_ISREG(fileinfo.st_mode))
		head->typeflag = '0';
	if (S_ISLNK(fileinfo.st_mode)){
        char sylinkname[100];
        head->typeflag = '1';
        if (readlink(file_path.c_str(), sylinkname, fileinfo.st_size) != 0)
			perror("link");
        strcpy(head->linkname, sylinkname);
        head->linkname[fileinfo.st_size] = '\0';
    }
	if (S_ISFIFO(fileinfo.st_mode))
		head->typeflag = '2';
	if (S_ISDIR(fileinfo.st_mode))
		head->typeflag = '3';   
    head->fileflag = '1';
    char padd[218] = {'\0'};
	stpcpy(head->padding, padd);
	return head;
}

void pack_class::pack(string source_dirname, int target_file, vector<string> filter_arg){
    // 获得从源目录外层开始的相对路径,写入头文件,便于还原到其他目录
    string filename,rel = ".";
    for (int i = source_dirname.size()-1; i > -1; i--)
        if (source_dirname[i] == '/'){
            filename = source_dirname.substr(i,source_dirname.size()-i);
            break;
        }
    // rel:从源路径(要打包的文件夹)外层开始的路径
    // 而输入的源路径是绝对路径或从可执行程序开始的路径,不一定等同于rel
    rel = rel + filename;
    char flag[BLOCKSIZE] = {'\0'};
    if(pack_class::packToFile(source_dirname, target_file, filter_arg, rel)){
        write(target_file, flag, BLOCKSIZE);
		close(target_file);
    }
}

bool pack_class::packToFile(string source_dirname, int target_file, vector<string> filter_arg, string rel){
    DIR *dir;
	struct dirent *entry;
	struct stat fileinfo;
	headblock *head;
    char buffer[BLOCKSIZE];
    // 如果最后一块不满,用于填满最后一块
    char flag[BLOCKSIZE] = {'\0'};
    head = getHeadBlock(source_dirname,rel);
    write(target_file, head, BLOCKSIZE);
    int fin, fout = target_file;
    // source_dirname 必须是目录
    if ((dir = opendir(source_dirname.c_str())) == NULL){
		return false;
	}
    while ((entry = readdir(dir)) != NULL){
        // 跳过.和..两个特殊的目录
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        // 获取该文件的路径
        string file_path = source_dirname + "/" + entry->d_name;
        string rel_path = rel + "/" + entry->d_name;
        if(lstat(file_path.c_str(),&fileinfo)!=0){
            printf("cannot create stat struct\n");
            exit(1);
        }
        if(S_ISDIR(fileinfo.st_mode)){
            packToFile(file_path, fout, filter_arg, rel_path);
        }
        else if(S_ISREG(fileinfo.st_mode)){
            if(!filter::filejudge(filter_arg,entry->d_name,file_path,fileinfo)) continue;
            head = getHeadBlock(file_path,rel_path);
            write(fout, head, BLOCKSIZE);
            fin = open(file_path.c_str(), O_RDONLY);
            if(fileinfo.st_size <= BLOCKSIZE){
                read(fin, buffer, fileinfo.st_size);
				write(fout, buffer, fileinfo.st_size);
                // 如果最后一块不满,填满最后一块
				write(fout, flag, BLOCKSIZE - fileinfo.st_size);
            }else{// 文件比一个块大,要写入多个块
                for (int i = 0; i < (fileinfo.st_size / BLOCKSIZE); i++)
				{
					read(fin, buffer, BLOCKSIZE);
					write(fout, buffer, BLOCKSIZE);
				}
                read(fin, buffer, fileinfo.st_size%BLOCKSIZE);
				write(fout, buffer, fileinfo.st_size%BLOCKSIZE);
                // 如果最后一块不满,填满最后一块
				write(fout, flag, BLOCKSIZE - fileinfo.st_size%BLOCKSIZE);
            }
            close(fin);
        }// 如果是链接文件和管道文件，只保留头结点
        else if(S_ISLNK(fileinfo.st_mode)||S_ISFIFO(fileinfo.st_mode)){
            head = getHeadBlock(file_path,rel_path);
			write(fout, head, BLOCKSIZE);
        }
    }
    closedir(dir);
    return true;
}

// 解包函数
bool pack_class::unpack(int source_file, string target_dirname){
    int fout;
	char buffer[BLOCKSIZE];
    timespec times[2];
    struct stat baginfo;
    // 假设最多有1024个文件
    headblock head[1024];
    while(read(source_file,head,BLOCKSIZE)){
        if (head->fileflag != '1')
			continue;
        times[0].tv_sec = time_t(atol(head->mtime_sec));
        times[0].tv_nsec = atol(head->mtime_nsec);
        times[1] = times[0];
        string filepath = target_dirname + '/' + head->name;
        if (head->typeflag == '0'){
            if ((fout = open(filepath.c_str(), O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0644)) < 0)
			{
				return false;
			}
            if(atol(head->size)<=BLOCKSIZE){
                read(source_file, buffer, atoi(head->size));
				write(fout, buffer, atoi(head->size));
                // 最后面为了补齐一个块的空内容,只读不写
				read(source_file, buffer, BLOCKSIZE - atoi(head->size));
            }else{
                for (int i = 0; i < (atoi(head->size) / BLOCKSIZE); i++){
                    read(source_file,buffer,BLOCKSIZE);
                    write(fout,buffer,BLOCKSIZE);
                }
                //读最后一个块
                read(source_file, buffer, atoi(head->size) % BLOCKSIZE);
				write(fout, buffer, atoi(head->size) % BLOCKSIZE);
				read(source_file, buffer, BLOCKSIZE - (atoi(head->size) % BLOCKSIZE));
            }
            close(fout);
        }
        else if(head->typeflag == '1'){
            if (symlink(head->linkname, filepath.c_str()) < 0)
			{
				perror("link");
				return false;
			}
        }
        else if(head->typeflag == '2'){
            if (mkfifo(filepath.c_str(), atoi(head->mode)) < 0)
			{
				perror("pipe");
				return false;
			}
        }
        else if(head->typeflag == '3'){
            mkdir(filepath.c_str(), atoi(head->mode));
        }
        chmod(filepath.c_str(), atoi(head->mode));
		chown(filepath.c_str(), atoi(head->uid), atoi(head->gid));
		utimensat(AT_FDCWD, filepath.c_str(), times, AT_SYMLINK_NOFOLLOW);
    }
    return true;
}