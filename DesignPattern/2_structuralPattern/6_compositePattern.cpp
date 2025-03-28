#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
using namespace std;

//抽象接口
class FileSystemComponent{
public:
    virtual void display(int depth = 0) const = 0;
    virtual size_t getSize() const = 0;
    virtual ~FileSystemComponent() = default;
};

// 叶子节点：文件
class File : public FileSystemComponent {
private:
    string name_;
    size_t size_;
public:
    File(const string& name,size_t size):name_(name),size_(size){}

    void display(int depth = 0) const override{
        cout << string(depth, '\t') << "📄 " << name_ 
            << " (" << size_ << " bytes)" << endl;
    }

    size_t getSize() const override { return size_; }
};

// 复合节点：目录
class Directory:public FileSystemComponent{
private:
    string name_;
    vector<shared_ptr<FileSystemComponent>> children_;
public:
    Directory(const string& name) : name_(name) {}

    void addComponent(shared_ptr<FileSystemComponent> component){
        children_.push_back(component);
    }

    void removeComponent(shared_ptr<FileSystemComponent> component){
        children_.erase(
            remove(children_.begin(), children_.end(), component),
            children_.end()
        );
    }

    void display(int depth = 0) const override {
        cout << string(depth, '\t') << "📁 " << name_ 
            << " [" << getSize() << " bytes]" << endl;

        for (const auto& child : children_) {
            child->display(depth + 1);
        }
    }

    size_t getSize() const override {
        size_t total = 0;
        for (const auto& child : children_) {
            total += child->getSize();
        }
        return total;
    }
};

int main(){
    // 创建文件
    auto file1 = make_shared<File>("document.txt", 1500);
    auto file2 = make_shared<File>("image.jpg", 2500);
    auto file3 = make_shared<File>("notes.md", 800);

    // 创建子目录
    auto subdir = make_shared<Directory>("Downloads");
    subdir->addComponent(file2);
    subdir->addComponent(file3);

    // 创建根目录
    auto root = make_shared<Directory>("Root");
    root->addComponent(file1);
    root->addComponent(subdir);

    // 添加另一个文件到根目录
    root->addComponent(make_shared<File>("backup.zip", 4200));

    // 显示整个结构
    cout << "File System Structure:\n";
    root->display();

    cout << "\nTotal size of root: " 
            << root->getSize() << " bytes" << endl;

    return 0;
}