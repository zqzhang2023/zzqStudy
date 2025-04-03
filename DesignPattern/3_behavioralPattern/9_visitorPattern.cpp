#include<iostream>
#include<vector>
using namespace std;

// 前置声明
class ConcreteElementA;
class ConcreteElementB;

// 抽象访问者
class Visitor {
public:
    virtual void visit(ConcreteElementA* element) = 0;
    virtual void visit(ConcreteElementB* element) = 0;
    virtual ~Visitor() {}
};

// 抽象元素
class Element {
public:
    virtual void accept(Visitor* visitor) = 0;
    virtual ~Element() {}
};

// 具体元素A
class ConcreteElementA : public Element {
public:
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
    string operationA() {
        return "具体元素A的操作";
    }
};

// 具体元素B
class ConcreteElementB : public Element {
public:
    void accept(Visitor* visitor) override {
        visitor->visit(this);
    }
    string operationB() {
        return "具体元素B的操作";
    }
};

// 具体访问者
class ConcreteVisitor : public Visitor {
public:
    void visit(ConcreteElementA* element) override {
        cout << "访问者正在访问 " << element->operationA() << endl;
    }
    
    void visit(ConcreteElementB* element) override {
        cout << "访问者正在访问 " << element->operationB() << endl;
    }
};


// 对象结构（管理元素集合）
class ObjectStructure {
private:
    vector<Element*> elements;
    
public:
    void add(Element* element) {
        elements.push_back(element);
    }
    
    void accept(Visitor* visitor) {
        for (auto elem : elements) {
            elem->accept(visitor);
        }
    }
};
int main(){

    ObjectStructure structure;
    structure.add(new ConcreteElementA());
    structure.add(new ConcreteElementB());
    
    ConcreteVisitor visitor;
    structure.accept(&visitor);

    return 0;
}