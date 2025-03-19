#include <iostream>
#include <fstream>
#include <string>
#include <cstring> // 用于字符串操作
using namespace std;
int main() {
    // 创建并写入文件
    ofstream outFile("example.txt");
    if (!outFile.is_open()) {
        cerr << "无法创建文件！" << endl;
        return 1;
    }

    outFile << "Hello, World!\n";
    outFile << "12345\n";
    outFile << "This is a test line with A character A\n";
    outFile << "Another line.\n";
    outFile.close();

    ifstream inFile("example.txt");
    if (!inFile.is_open()) {
        cerr << "无法打开文件！" << endl;
        return 1;
    }

    // 使用 file.getline(buffer, size)
    char buffer[100];
    cout << "使用 file.getline(buffer, size) 读取一行：" << endl;
    inFile.getline(buffer, 100);
    cout << buffer << endl;

    // 使用 file.read(buffer, size)
    char buffer2[10];
    cout << "\n使用 file.read(buffer, size) 读取 10 个字符：" << endl;
    inFile.read(buffer2, 10);
    buffer2[9] = '\0'; // 确保字符串以空字符结尾
    cout << buffer2 << endl;

    // 使用 file >> x 读取一个 int 值
    int x;
    cout << "\n使用 file >> x 读取一个 int 值：" << endl;
    inFile >> x;
    cout << "读取的整数是：" << x << endl;

    // 使用 file.get() 读取一个字符
    char ch = inFile.get();
    cout << "\n使用 file.get() 读取一个字符：" << endl;
    cout << "读取的字符是：" << ch << endl;

    // 使用 file.get(x) 读取一个字符到变量中
    char ch2;
    cout << "\n使用 file.get(x) 读取一个字符到变量中：" << endl;
    inFile.get(ch2);
    cout << "读取的字符是：" << ch2 << endl;

    // 使用 file.get(buffer, num, delim)
    char str1[127];
    cout << "\n使用 file.get(buffer, num, delim) 读取字符到字符串，直到遇到 'A'：" << endl;
    inFile.get(str1, 127, 'A');
    cout << "读取的字符串是：" << str1 << endl;

    inFile.close();
    return 0;
}