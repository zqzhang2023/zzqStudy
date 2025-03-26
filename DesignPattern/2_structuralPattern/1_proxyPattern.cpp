#include<iostream>
#include<string>
using namespace std;

// 抽象主题接口
class Image{
public:
    virtual void display() = 0;
    virtual ~Image() = default;
};

// 真实主题类
class RealImage: public Image{
private:
    string filename;
    // 从磁盘之中加载图片
    void loadFromDisk() {
        cout << "Loading image: " << filename << " from disk" << endl;
    }
public:
    RealImage(const string& filename):filename(filename){
        loadFromDisk();
    };

    void display() override {
        cout << "Displaying image: " << filename << endl;
    }

};

// 代理类
class ProxyImage :public Image{
private:
    RealImage* realImage = nullptr;
    string filename;
    bool accessAllowed = true; // 简单的访问控制标志

    bool checkAccess() const {
        // 这里可以添加更复杂的访问控制逻辑
        return accessAllowed;
    }

    void logAccess() {
        cout << "Logged access to image: " << filename << endl;
    }
    
public:
    ProxyImage(const string& filename) : filename(filename) {}

    void display() override {
        if (!checkAccess()) {
            std::cout << "Access denied for image: " << filename << std::endl;
            return;
        }

        if (realImage == nullptr) {
            realImage = new RealImage(filename); // 延迟初始化
        }
        realImage->display();
        logAccess();
    }

    ~ProxyImage() {
        delete realImage;
    }

};



int main(){

    Image* image1 = new ProxyImage("photo1.jpg");
    Image* image2 = new ProxyImage("photo2.jpg");

    // 第一次访问会加载图片
    image1->display(); 

    // 第二次访问直接显示（已加载）
    image1->display();

    image2->display();

    delete image1;
    delete image2;
    
    return 0;
}