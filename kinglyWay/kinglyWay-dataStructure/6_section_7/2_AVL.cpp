#include<iostream>
#include<algorithm>
using namespace std;

struct AVLNode{
    int val;
    int height;
    AVLNode* left;
    AVLNode* right;
    AVLNode(int x):val(x),height(1),left(nullptr),right(nullptr){};
};

class AVLTree{
    AVLNode* root;

    // 获取节点高度
    int getHeight(AVLNode* node){
        return node ? node->height : 0;
    }

    // 更新节点的高度
    void updateHeight(AVLNode* node){
        if(node){
            node->height = 1 + max(getHeight(node->left),getHeight(node->right));
        }
    }

    // 计算平衡因子
    int getBalanceFactor(AVLNode* node) {
        return getHeight(node->left) - getHeight(node->right);
    }

    // 右旋操作
    AVLNode* rightRotate(AVLNode* y){
        AVLNode* x = y->left;
        AVLNode* T3 = x->right;

        x->right = y;
        y->left = T3;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    // 左旋操作
    AVLNode* leftRotate(AVLNode* x){
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    // 插入节点
    AVLNode* insertNode(AVLNode* node, int val) {
        if(!node){
            return new AVLNode(val);
        }

        if(val < node->val){
            node->left = insertNode(node->left,val);
        }else if(val > node->val){
            node->right = insertNode(node->right,val);
        }else{
            return node; //不允许重复的值
        }

        updateHeight(node);
        int balance = getBalanceFactor(node);


        // 左左情况 右单旋
        if (balance > 1 && val < node->left->val)
            return rightRotate(node);

        // 右右情况 左单旋
        if (balance < -1 && val > node->right->val)
            return leftRotate(node);

        // 左右情况 左边先左旋 然后整体再右旋
        if (balance > 1 && val > node->left->val) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        return node;
    }


    // 删除节点（递归）
    AVLNode* deleteNode(AVLNode* node, int val) {
        if (!node) return nullptr;

        if (val < node->val){
            node->left = deleteNode(node->left, val);
        }else if (val > node->val){
            node->right = deleteNode(node->right, val);
        }else{
            //找到待删除的节点
            if(!node->left||!node->right){ //子树有空的
                AVLNode* temp = node->left ? node->left : node->right;
                if(!temp){ // 都为空 直接删除node
                    temp = node;
                    node = nullptr;
                }else{
                    *node = *temp; // 直接替换内容
                }
                delete temp;
            }else{ // 子树都不为空
                //和BST一样找到右边最小的节点
                AVLNode* minRight = node->right;
                while(minRight->left){
                    minRight = minRight->left;
                }
                node->val = minRight->val;
                node->right = deleteNode(node->right,minRight->val);
            }
        }

        if (!node) return nullptr;

        // 更新高度并平衡
        updateHeight(node);
        int balance = getBalanceFactor(node);


        // 右左情况 右边先右旋 然后整体再右旋
        if (balance < -1 && val < node->right->val) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        // 左左情况 右单旋
        if (balance > 1 && getBalanceFactor(node->left) >= 0)
            return rightRotate(node);

        // 左右情况 左边先左旋 然后整体再右旋
        if (balance > 1 && getBalanceFactor(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // 右右情况 左单旋
        if (balance < -1 && getBalanceFactor(node->right) <= 0)
            return leftRotate(node);

        // 右左情况 右边先右旋 然后整体再右旋
        if (balance < -1 && getBalanceFactor(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    // 中序遍历辅助函数
    void inorderTraversal(AVLNode* node) {
        if (!node) return;
        inorderTraversal(node->left);
        cout << node->val << " ";
        inorderTraversal(node->right);
    }

public:
    AVLTree() : root(nullptr) {}

    // 插入接口
    void insert(int val) {
        root = insertNode(root, val);
    }

    // 删除接口
    void remove(int val) {
        root = deleteNode(root, val);
    }

    // 查找接口
    bool search(int val) {
        AVLNode* curr = root;
        while (curr) {
            if (curr->val == val) return true;
            else if (val < curr->val) curr = curr->left;
            else curr = curr->right;
        }
        return false;
    }

    // 打印中序遍历结果
    void printInorder() {
        inorderTraversal(root);
        cout << endl;
    }

};

int main(){
    AVLTree avl;

    // 插入测试
    avl.insert(10);
    avl.insert(20);
    avl.insert(30);
    avl.insert(40);
    avl.insert(50);
    avl.insert(25);

    cout << "插入后的中序遍历: ";
    avl.printInorder(); // 10 20 25 30 40 50 

    // 查找测试
    cout << "查找25: " << (avl.search(25) ? "存在" : "不存在") << endl; // 存在
    cout << "查找100: " << (avl.search(100) ? "存在" : "不存在") << endl; // 不存在

    // 删除测试
    avl.remove(30); // 删除根节点
    cout << "删除30后的中序遍历: ";
    avl.printInorder(); // 10 20 25 40 50 

    avl.remove(25); // 删除叶子节点
    cout << "删除25后的中序遍历: ";
    avl.printInorder(); // 10 20 40 50 

    return 0;
}



