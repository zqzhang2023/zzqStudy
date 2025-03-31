#include<iostream>
#include <string>
#include <unordered_map>
#include <vector>
using namespace std;

// 享元对象：包含树木的固有属性（内部状态）
class TreeType {
private:
    string name_;   // 内部状态
    string color_;  // 内部状态
    string texture_; // 内部状态
public:
    TreeType(const string& name, const string& color, const string& texture)
        : name_(name), color_(color), texture_(texture) {}
    
    void draw(int x, int y, int age) const { // 外部状态作为参数
        cout << "绘制 " << name_ << " 在 (" << x << ", " << y 
                  << ")，颜色：" << color_ 
                  << "，纹理：" << texture_
                  << "，年龄：" << age << "年\n";
    }
};

// 享元工厂
class TreeFactory {
private:
    unordered_map<string,TreeType> treeTypes_;
    // 生成唯一键
    string makeKey(const string& name, 
                const string& color, 
                const string& texture) {
        return name + "_" + color + "_" + texture;
    }
public:
    const TreeType& getTreeType(const string& name, 
                               const string& color, 
                               const string& texture) {
        string key = makeKey(name, color, texture);
        // 如果不存在则创建新类型
        if (treeTypes_.find(key) == treeTypes_.end()) {
            cout << "创建新的树木类型: " << key << endl;
            treeTypes_.emplace(key, TreeType(name, color, texture));
        }
        return treeTypes_.at(key);
    }
};

// 包含外部状态的树木对象
class Tree {
private:
    int x_;         // 外部状态
    int y_;         // 外部状态
    int age_;       // 外部状态
    const TreeType& type_; // 指向享元的引用

public:
    Tree(int x, int y, int age, const TreeType& type)
        : x_(x), y_(y), age_(age), type_(type) {}

    void draw() const {
        type_.draw(x_, y_, age_);
    }
};


// 森林包含多个树木
class Forest {
private:
    vector<Tree> trees_;
    TreeFactory factory_;

public:
    void plantTree(int x, int y, int age,
                   const string& name,
                   const string& color,
                   const string& texture) {
        const TreeType& type = factory_.getTreeType(name, color, texture);
        trees_.emplace_back(x, y, age, type);
    }

    void draw() const {
        for (const auto& tree : trees_) {
            tree.draw();
        }
    }
};



int main() {
    Forest forest;
    
    // 种植不同类型的树
    forest.plantTree(1, 2, 5, "松树", "深绿", "针叶纹理");
    forest.plantTree(3, 4, 7, "橡树", "浅绿", "宽叶纹理");
    forest.plantTree(5, 6, 3, "松树", "深绿", "针叶纹理"); // 复用已有类型
    forest.plantTree(7, 8, 2, "白桦", "白色", "条纹纹理");
    forest.plantTree(9, 0, 4, "橡树", "浅绿", "宽叶纹理"); // 复用已有类型

    cout << "\n开始绘制森林：\n";
    forest.draw();

    return 0;
}