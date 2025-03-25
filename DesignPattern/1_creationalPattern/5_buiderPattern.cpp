#include<iostream>
#include<string>
using namespace std;

// 产品类：Computer
class Computer{
private:
    string m_cpu;
    string m_memory;
    string m_hardDisk;
public:
    void setCPU(const string& cpu){ m_cpu = cpu;}
    void setMemory(const string& memory) { m_memory = memory; }
    void setHardDisk(const string& hardDisk) { m_hardDisk = hardDisk; }

    void show() const {
        cout << "Computer Configuration:" << endl;
        cout << "CPU: " << m_cpu << endl;
        cout << "Memory: " << m_memory << endl;
        cout << "Hard Disk: " << m_hardDisk << endl;
    }
};

// 抽象建造者接口
class ComputerBuilder {
public:
    virtual void buildCPU() = 0;
    virtual void buildMemory() = 0;
    virtual void buildHardDisk() = 0;
    virtual Computer* getResult() = 0;
    virtual ~ComputerBuilder() = default;
};


//  具体建造者：高端配置
class HighEndComputerBuilder : public ComputerBuilder {
private:
    Computer* m_computer;
public:
    HighEndComputerBuilder() : m_computer(new Computer()) {}

    void buildCPU() override {
        m_computer->setCPU("Intel Core i9-12900K");
    }

    void buildMemory() override {
        m_computer->setMemory("64GB DDR5 4800MHz");
    }

    void buildHardDisk() override {
        m_computer->setHardDisk("2TB Samsung 980 Pro NVMe SSD");
    }

    Computer* getResult() override {
        return m_computer;
    }

    ~HighEndComputerBuilder() {
        delete m_computer;
    }

};

// 具体建造者：办公配置
class OfficeComputerBuilder : public ComputerBuilder {
private:
    Computer* m_computer;

public:
    OfficeComputerBuilder() : m_computer(new Computer()) {}

    void buildCPU() override {
        m_computer->setCPU("Intel Core i5-12400");
    }

    void buildMemory() override {
        m_computer->setMemory("16GB DDR4 3200MHz");
    }

    void buildHardDisk() override {
        m_computer->setHardDisk("512GB Crucial P2 NVMe SSD");
    }

    Computer* getResult() override {
        return m_computer;
    }

    ~OfficeComputerBuilder() {
        delete m_computer;
    }
};

// 指挥者类
class Director{
public:
    void construct(ComputerBuilder* builder){
        builder->buildCPU();
        builder->buildMemory();
        builder->buildHardDisk();
    }
};


int main(){

    Director director;

    // 构建高端电脑
    HighEndComputerBuilder highEndBuilder;
    director.construct(&highEndBuilder);
    Computer* gamingPC = highEndBuilder.getResult();
    cout << "High-end Gaming PC:" << endl;
    gamingPC->show();

    cout << "\n";

    // 构建办公电脑
    OfficeComputerBuilder officeBuilder;
    director.construct(&officeBuilder);
    Computer* officePC = officeBuilder.getResult();
    cout << "Office PC:" << endl;
    officePC->show();

    return 0;
}