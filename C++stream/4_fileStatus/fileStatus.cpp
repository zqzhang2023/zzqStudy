#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main() {
    ifstream file("example.txt"); // 尝试打开文件

    // 检查文件是否成功打开
    if (!file.is_open()) {
        cerr << "无法打开文件！" << endl;
        return 1;
    }

    cout << "文件已成功打开！" << endl;

    string line;
    int lineCount = 0;

    // 逐行读取文件内容
    while (!file.eof()) { // 判断是否到达文件末尾
        getline(file, line);

        // 检查读取是否成功
        if (file.fail()) {
            cerr << "读取文件时出错！" << endl;
            break;
        }

        if (!line.empty()) {
            lineCount++;
            cout << "第 " << lineCount << " 行: " << line << endl;
        }
    }

    // 检查文件是否正常关闭
    file.close();
    if (file.bad()) {
        cerr << "关闭文件时出错！" << endl;
        return 1;
    }

    // 模拟一个错误状态
    file.clear(); // 清除之前的错误状态
    file.setstate(ios::failbit); // 设置失败状态
    if (file.fail()) {
        cerr << "文件状态已设置为失败！" << endl;
    }

    // 检查文件状态
    if (file.good()) {
        cout << "文件状态良好！" << endl;
    } else {
        cerr << "文件状态异常！" << endl;
    }

    return 0;
}