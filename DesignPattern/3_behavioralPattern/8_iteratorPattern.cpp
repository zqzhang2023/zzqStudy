#include<iostream>
#include<string>
using namespace std;

// 前向声明
class StringCollection;

// 迭代器接口
class Iterator{
public:
    virtual string next() = 0;
    virtual bool hasNext() const = 0;
    virtual ~Iterator() = default;
};

// 集合接口
class Collection {
public:
    virtual ~Collection() = default;
    virtual Iterator* createIterator() const = 0;
};


class StringIterator : public Iterator {
private:
    const StringCollection& collection;
    size_t currentIndex;
public:
    StringIterator(const StringCollection& coll);
    string next() override;
    bool hasNext() const override;
};

// 具体集合类
class StringCollection : public Collection {
private:
    string items[5];
    size_t count = 0;

public:
    void add(const string& item) {
        if (count < 5) {
            items[count++] = item;
        }
    }

    size_t size() const { return count; }
    string get(size_t index) const { return items[index]; }

    Iterator* createIterator() const override {
        return new StringIterator(*this);
    }
};

// 具体迭代器实现
StringIterator::StringIterator(const StringCollection& coll)
    : collection(coll), currentIndex(0) {}

string StringIterator::next() {
    return collection.get(currentIndex++);
}

bool StringIterator::hasNext() const {
    return currentIndex < collection.size();
}

int main(){

    StringCollection collection;
    collection.add("First");
    collection.add("Second");
    collection.add("Third");

    Iterator* it = collection.createIterator();
    
    while (it->hasNext()) {
        cout << it->next() << endl;
    }

    delete it;
    
    return 0;
}