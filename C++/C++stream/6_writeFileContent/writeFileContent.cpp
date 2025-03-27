#include <iostream>
#include <fstream>
#include <cstring> // 用于字符串操作
using namespace std;

int main() {
    // 创建并打开文件用于写入
    ofstream outFile("output.txt");
    if (!outFile.is_open()) {
        cerr << "无法打开文件！" << endl;
        return 1;
    }

    // 使用 file << "xxxxx" << endl; 写入字符串
    outFile << "Hello, World!" << endl;
    outFile << "This is a test file." << endl;

    // 使用 file.put('a'); 写入单个字符
    outFile.put('a');
    outFile.put('b');
    outFile.put('c');
    outFile << endl; // 换行

    // 使用 file.write(buffer, counter); 写入缓冲区中的数据
    const char* buffer = "This is written using write()";
    int bufferSize = strlen(buffer); // 获取字符串长度
    outFile.write(buffer, bufferSize); // 写入缓冲区中的数据

    // 写入一些额外内容
    outFile << endl;
    outFile << "Another line added at the end." << endl;

    // 关闭文件
    outFile.close();

    cout << "文件写入完成！" << endl;

    return 0;
}