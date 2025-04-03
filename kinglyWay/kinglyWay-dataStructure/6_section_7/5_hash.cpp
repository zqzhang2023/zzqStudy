#include<iostream>
#include<string>
#include<vector>
using namespace std;

const int TABLE_SIZE = 10; // 固定大小的哈希表

struct HashNode{
    string key;
    int value;
    HashNode* next;
    HashNode(const string& k,int v):key(k),value(v),next(nullptr){}
};

class HashTable{
private:
    vector<HashNode*> buckets; // 桶数组

    // 哈希函数（简单示例）
    int hashFunction(const string& key){
        int hash = 0;
        for(char c:key){
            hash += c;
        }
        return hash % TABLE_SIZE;
    }
public:
    HashTable() : buckets(TABLE_SIZE, nullptr) {}
    ~HashTable(){
        // 析构时释放所有节点
        for(int i = 0; i < TABLE_SIZE; ++i){
            HashNode* entry = buckets[i];
            while(entry){
                HashNode* prev = entry;
                entry = entry->next;
                delete entry;
            }
        }
    }

    // 插入键值对
    void insert(const string& key, int value) {
        int bucketIndex = hashFunction(key);
        HashNode* entry = buckets[bucketIndex];
        while (entry){
            if (entry->key == key) {
                entry->value = value; // 更新现有值
                return;
            }
            entry = entry->next;
        }
        // 创建新节点并插入链表头部
        HashNode* newNode = new HashNode(key, value);
        newNode->next = buckets[bucketIndex]; //头插法
        buckets[bucketIndex] = newNode;
    }

    // 获取值
    int get(const string& key) {
        int bucketIndex = hashFunction(key);
        HashNode* entry = buckets[bucketIndex];
        while(entry){
            if(entry->key == key){
                return entry->value;
            }
            entry = entry->next;
        }
        return -1;// 未找到返回-1
    }

    // 删除键
    void remove(const string& key){
        int bucketIndex = hashFunction(key);
        HashNode* entry = buckets[bucketIndex];
        HashNode* prev = nullptr;
        while(entry){
            if(entry->key == key){
                if(prev){
                    prev->next = entry->next;
                }else{
                    buckets[bucketIndex] = entry->next;
                }
                delete entry;
            }
            prev = entry;
            entry = entry->next;
        }
    }
};

int main() {
    HashTable myHashTable;

    myHashTable.insert("Alice", 30);
    myHashTable.insert("Bob", 25);
    myHashTable.insert("Charlie", 28);

    cout << "Alice's age: " << myHashTable.get("Alice") << endl; // 30
    cout << "Unknown key: " << myHashTable.get("Dave") << endl;   // -1

    myHashTable.remove("Bob");
    cout << "After removal - Bob: " << myHashTable.get("Bob") << endl; // -1

    return 0;
}