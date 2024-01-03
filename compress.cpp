#include"./headFiles/compress.h"

void compress_class::encode(haffNode *pn, string code)
{
    pn->code = code;
    if (pn->left)
        encode(pn->left, code + "0");
    if (pn->right)
        encode(pn->right, code + "1");
    if (!pn->left && !pn->right)
    {
        codeMap[pn->uchar] = code;
    }
}

bool compress_class::compress(string sourcefile, string targetfile){
    codeMap.clear();
    ifstream inFile;
    ofstream outFile;
    inFile.open(sourcefile, ios::in | ios::binary);
    string compress_file_path = targetfile;
    outFile.open(compress_file_path, ios::out | ios::binary);

    // 统计词频,储存在map中
    // map的key为字符,按照字符char的字典序排序
    unsigned char uchar;
    map<unsigned char, unsigned long long> freqMap;
    while (inFile.read((char *)&uchar, sizeof(char)))
    {
        freqMap[uchar]++;
    }

    //优先队列自动按照词频排序
    priority_queue<haffNode *, vector<haffNode *>, Compare> freqHeap;
    for (auto it = freqMap.begin(); it != freqMap.end(); it++)
    {
        haffNode *pn = new (haffNode);
        pn->freq = it->second;
        pn->uchar = it->first;
        pn->left = pn->right = 0;
        freqHeap.push(pn);
    }
    
    //构建哈夫曼树
    while (freqHeap.size() > 1){
        haffNode *pn1 = freqHeap.top();
        freqHeap.pop();
        haffNode *pn2 = freqHeap.top();
        freqHeap.pop();
        haffNode *pn = new (haffNode);
        pn->freq = pn1->freq + pn2->freq;
        pn->left = pn1;
        pn->right = pn2;
        freqHeap.push(pn);
    }
    haffNode *root = freqHeap.top();
    codeMap.clear();

    //用生成的哈夫曼树编码
    encode(root, "");

    const unsigned char zeroUC = 0;
    outFile.write((char *)&zeroUC, sizeof(zeroUC));

    // 频率表写入压缩文件头部
    const unsigned long long zeroULL = 0;
    int i = 0;
    while (i < 256)
    {
        if (freqMap.count(i) == 0)
        {
            outFile.write((char *)&zeroULL, sizeof(zeroULL));
        }
        else
        {
            unsigned long long freq = freqMap[i];
            outFile.write((char *)&freq, sizeof(freq));
        }
        i++;
    }

    // 写入压缩文件主体,最后补上补零数
    {
        inFile.clear();
        inFile.seekg(0);
        string buf;
        unsigned char uchar;
        while (inFile.read((char *)&uchar, sizeof(uchar)))
        {
            buf += codeMap[uchar];
            while (buf.length() >= 8)
            {
                bitset<8> bs(buf.substr(0, 8));
                uchar = bs.to_ulong();
                outFile.write((char *)&uchar, sizeof(uchar));
                buf = buf.substr(8);
            }
        }
        // 末尾处理
        int zeroNum = (8 - buf.length()) % 8;
        if (zeroNum)
        {
            for (int i = 0; i < zeroNum; i++)
            {
                buf += "0";
            }
            bitset<8> bs(buf.substr(0, 8));
            uchar = bs.to_ulong();
            outFile.write((char *)&uchar, sizeof(uchar));
        }
        // 写入头部预留的补零数
        outFile.clear();// 清除文件流的状态
        outFile.seekp(0);
        uchar = zeroNum;
        outFile.write((char *)&uchar, sizeof(uchar));
    }
    inFile.close();
    outFile.close();
    // 正常执行
    return true;
}

// 解压函数
bool compress_class::decompress(string sourcefile, string targetfile){
    if (sourcefile.substr(sourcefile.find_last_of(".") + 1) != "comp")
    {
        cout << "源文件格式不正确\n";
        return false;
    }
    ifstream inFile;
    ofstream outFile;
    inFile.open(sourcefile, ios::in | ios::binary);
    string decompress_file_path = targetfile;
    outFile.open(decompress_file_path, ios::out | ios::binary);

    unsigned char uchar;
    inFile.read((char *)&uchar, sizeof(char));
    int zeroNum = uchar;

    unsigned long long freq;
    map<unsigned char, unsigned long long> freqMap;
    int i = 0;
    for (i = 0; i < 256; i++)
    {
        inFile.read((char *)&freq, sizeof(freq));
        if (freq)
        {
            freqMap[i] = freq;
        }
    }

    if (i != 256)
    {
        cout << "文件过短\n";
        return false; // 文件过短，频率表不完整
    }

    // 建立词频小顶堆
    priority_queue<haffNode *, vector<haffNode *>, Compare> freqHeap;
    for (auto it = freqMap.begin(); it != freqMap.end(); it++)
    {
        haffNode *pn = new (haffNode);
        pn->freq = it->second;
        pn->uchar = it->first;
        pn->left = pn->right = 0;
        freqHeap.push(pn);
    }
    
    /**构建哈夫曼树**/
    while (freqHeap.size() > 1)
    {
        haffNode *pn1 = freqHeap.top();
        freqHeap.pop();
        haffNode *pn2 = freqHeap.top();
        freqHeap.pop();
        haffNode *pn = new (haffNode);
        pn->freq = pn1->freq + pn2->freq;
        pn->left = pn1;
        pn->right = pn2;
        freqHeap.push(pn);
    }
    haffNode *root = freqHeap.top();
    codeMap.clear();

    // 读出主体，用哈夫曼树树解码
    haffNode *decodePointer = root;
    string buf, now;
    inFile.read((char *)&uchar, sizeof(unsigned char));

    bitset<8> bs = uchar;
    buf = bs.to_string();
    while (inFile.read((char *)&uchar, sizeof(unsigned char))){

        bitset<8> bs = uchar;
        now = buf;
        buf = bs.to_string();
        for (char i = 0; i < 8; i++)
        {
            if (now[i] == '0')
            {
                if (!decodePointer->left)
                {
                    cout << "解码错误\n";
                    return false; // 解码错误
                }
                decodePointer = decodePointer->left;
            }
            else
            {
                if (!decodePointer->right)
                {
                    cout << "解码错误\n";
                    return false; // 解码错误
                }
                decodePointer = decodePointer->right;
            }
            if (!(decodePointer->left || decodePointer->right))
            {
                // cout<<int(decodePointer->uchar)<<" ";
                outFile.write((char *)&(decodePointer->uchar), sizeof(decodePointer->uchar));
                decodePointer = root;
            }
        }
    }
    now = buf;
    for (char i = 0; i < (8 - zeroNum) % 8; i++)
    {
        if (now[i] == '0')
        {
            if (!decodePointer->left)
            {
                cout << "解码错误\n";
                return false; // 解码错误
            }
            decodePointer = decodePointer->left;
        }
        else
        {
            if (!decodePointer->right)
            {
                cout << "解码错误\n";
                return false; // 解码错误
            }
            decodePointer = decodePointer->right;
        }
        if (!(decodePointer->left || decodePointer->right))
        {
            // cout<<int(decodePointer->uchar)<<" ";
            outFile.write((char *)&(decodePointer->uchar), sizeof(unsigned char));
            decodePointer = root;
        }
    }
    inFile.close();
    outFile.close();
    if (decodePointer == root)
        return true; // 正常执行
    else
    {
        cout << "结尾不完整\n";
        return false; // 文件结尾不完整
    }
}