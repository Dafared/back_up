#include "./headFiles/filter.h"

// 判断自定义备份文件大小条件
bool filter::sizejudge(off_t byte_size, string match_size){
    if(match_size=="0") return true;
    long long msize = stoll(match_size.substr(1,match_size.size()-1));
    if(match_size[0]=='s'){
        return byte_size<=msize;
    }
    else{
        return byte_size>=msize;
    }
}

// 判断自定义备份文件类型条件
bool filter::typejudge(string type, string types){
    if(types=="0") return true;
    // 把自定义需要备份的类型名保存进unordered_set
    unordered_set<string> matchType;
    for(int i=0,j=0;i<types.size();i++){
        if(types[i]=='/'){
            matchType.emplace(types.substr(j,i-j));
            j += (i-j+1);
        }
    }
    return matchType.find(type)!=matchType.end();
}

// 判断自定义备份修改时间条件
bool filter::timejudge(time_t time, string match_time){
    if(match_time=="0") return true;
    struct tm tm_match;
    string timestring = match_time.substr(1,match_time.size()-1);
    strptime(timestring.c_str(), "%Y-%m-%d %H:%M:%S",  &tm_match);
    time_t timeStamp = mktime(&tm_match);
    if(match_time[0]=='s'){
        return time<=timeStamp;
    }else{
        return time>=timeStamp;
    }
}

// 判断自定义备份文件名条件
bool filter::namejudge(string name, string match_name){
    if(match_name=="0") return true;
    return name == match_name;
}

// 判断自定义备份所属路径条件
bool filter::pathjudge(string path, string match_path){
    if(match_path=="0") return true;
    char buf[2048];
    realpath(path.c_str(),buf);
    string path_ab = buf;
    realpath(match_path.c_str(),buf);
    string match_path_ab = buf;
    if(match_path_ab.size()>path_ab.size()) return false;
    for(int i=0;i<match_path_ab.size();i++){
        if(match_path_ab[i]!=path_ab[i]) return false;
    }
    return true;
}

// 判断文件是否符合filter_arg给定条件
bool filter::filejudge(vector<string> filter_arg, string type, off_t byte_size, string name, time_t time, string path){
    bool typematch,sizematch,namematch,timematch,pathmatch,match;
    typematch = typejudge(type,filter_arg[0]);
    sizematch = sizejudge(byte_size,filter_arg[1]);
    namematch = namejudge(name,filter_arg[2]);
    timematch = timejudge(time,filter_arg[3]);
    pathmatch = pathjudge(path,filter_arg[4]);
    match = typematch&&sizematch&&namematch&&timematch&&pathmatch;
    return match;
}

// 判断文件是否符合filter_arg给定条件
bool filter::filejudge(vector<string> filter_arg, string str_name, string file_path, struct stat fileinfo){
    string type;
    if(S_ISLNK(fileinfo.st_mode)){
        type = "link";
    }
    else if(S_ISFIFO(fileinfo.st_mode)){
        type = "pipe";
    }
    else{
        int point_pos = str_name.rfind('.');
        size_t type_len = str_name.size()-point_pos-1;
        type = str_name.substr(point_pos+1,type_len);
    }
    time_t m_time = fileinfo.st_mtime;
    off_t byte_size = fileinfo.st_size;
    return filejudge(filter_arg, type, byte_size, str_name, m_time, file_path);
}

// 用于检查用户输入的自定义备份条件格式是否正确
bool filter::checkfilter(char** argv) {
    if (argv[4][0] != '0') {    
        string s{argv[4]};
        if (s.size() < 2) {
            return false;
        }
        if (s[s.size()-1] != '/') {
            return false;
        }
    }
    if (argv[5][0] != '0' && argv[5][0] != 'b' && argv[5][0] != 's') {
        return false;
    }
    if (argv[7][0] != '0' && argv[7][0] != 'b' && argv[7][0] != 's') {
        return false;
    }
    if (argv[7][0] != '0' && !checkdatetime(argv[7])) {
        return false;
    }

    return true;
}

// 检查输入的日期时间格式是否正确
bool filter::checkdatetime(string str) {
    // 定义正则表达式，匹配形如“2021-12-01 12:00:00”的格式
    str.erase(str.begin());
    regex pattern(R"((\d{4})-(\d{2})-(\d{2}) (\d{2}):(\d{2}):(\d{2}))");
    smatch match;
    // 如果字符串不符合正则表达式，返回false
    if (!std::regex_match(str, match, pattern)) {
        return false;
    }
    // 提取字符串中的年月日时分秒
    int year = stoi(match[1]);
    int month = stoi(match[2]);
    int day = stoi(match[3]);
    int hour = stoi(match[4]);
    int minute = stoi(match[5]);
    int second = stoi(match[6]);
    // 使用tm结构体来验证日期和时间的有效性
    tm tm;
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    tm.tm_isdst = -1;
    // 尝试将tm结构体转换为time_t类型，如果失败，返回false
    time_t t = std::mktime(&tm);
    if (t == -1) {
        return false;
    }
    // 使用std::chrono库来比较tm结构体和time_t类型是否一致，如果不一致，返回false
    chrono::system_clock::time_point tp = chrono::system_clock::from_time_t(t);
    time_t t2 = std::chrono::system_clock::to_time_t(tp);
    if (std::difftime(t, t2) != 0.0) {
        return false;
    }
    // 如果以上都通过，返回true
    return true;
}

// 用于测试自定义条件过滤器的函数
void filter::testfilter(string test_file, vector<string> filter_arg) {
    bool result = false;
    struct stat fileinfo;
    if(lstat(test_file.c_str(),&fileinfo)!=0){
        cout<<test_file<<endl;
        printf("cannot create stat struct\n");
        exit(1);
    }
    string filename;
    for (int i = test_file.size()-1; i > -1; i--){
        if (test_file[i] == '/'){
            filename = test_file.substr(i+1,test_file.size()-i-1);
            break;
        }
    }
    if(!S_ISDIR(fileinfo.st_mode)){
        result = filter::filejudge(filter_arg,filename,test_file,fileinfo);
    }
    if (result) {
        cout<< "match filter condition"<<endl;
    } else {
        cout<< "not match filter condition"<<endl;
    }
}