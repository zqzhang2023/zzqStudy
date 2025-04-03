#include<iostream>
using namespace std;

struct TreeNode{
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int x):val(x),left(nullptr),right(nullptr){};
};

class BST{
private:
    TreeNode* root;
    //递归插入节点
    TreeNode* insertNode(TreeNode* node,int val){
        if(!node){
            return new TreeNode(val);
        }
        if(val < node->val){
            node->left = insertNode(node->left,val);
        }else if(val > node->val){
            node->right = insertNode(node->right,val);
        }
        //忽略重复的值
        return node;
    }

    //辅助函数：找到子树的最小节点
    TreeNode* findMin(TreeNode* node) {
        while (node && node->left) node = node->left;
        return node;
    }


    //递归删除节点
    TreeNode* deleteNode(TreeNode* node,int val){
        if(!node){
            return nullptr;
        }
        if(val < node->val){
            node->left = deleteNode(node->left,val);
        }else if(val > node->val){
            node->right = deleteNode(node->right,val);
        }else{
            // 情况1：无左子结点
            if(!node->left){
                TreeNode* right = node->right;
                delete node;
                return right;
            }
            // 情况2: 无右子节点
            else if (!node->right){ 
                TreeNode* left = node->left;
                delete node;
                return left;
            }
            // 情况3: 有两个子节点
            else{
                //找到右边的最小的节点
                TreeNode* minRight = findMin(node->right);
                //替换值并删除原始节点
                node->val = minRight->val;
                node->right = deleteNode(node->right,minRight->val);
            }
        }
        return node;
    }

    //中序遍历
    void inorderTraversal(TreeNode* node){
        if (!node) return;
        inorderTraversal(node->left);
        cout << node->val << " ";
        inorderTraversal(node->right);
    }

    // 递归释放节点
    void destroyTree(TreeNode* node) {
        if (!node) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }

public:
    BST() : root(nullptr) {}

    // 插入接口
    void insert(int val) {
        root = insertNode(root, val);
    }

    // 删除接口
    void remove(int val){
        root = deleteNode(root,val);
    }

    // 查找接口
    bool search(int val){
        TreeNode* curr = root;
        while (curr){
            if(curr->val == val){
                return true;
            }else if (curr->val < val){
                curr = curr->right;
            }else{
                curr = curr->left;
            }
        }
        return false;   
    }

    // 打印中序遍历的结果
    void printInorder(){
        inorderTraversal(root);
        cout<<"END"<<endl;
    }

    // 析构函数
    ~BST() {
        destroyTree(root);
    }

};

int main(){

    BST bst;
    
    // 插入示例
    bst.insert(5);
    bst.insert(3);
    bst.insert(7);
    bst.insert(2);
    bst.insert(4);
    bst.insert(6);
    bst.insert(8);

    cout << "初始中序遍历: ";
    bst.printInorder(); // 2 3 4 5 6 7 8 

    // 查找示例
    cout << "查找 4: " << (bst.search(4) ? "存在" : "不存在") << endl; // 存在
    cout << "查找 9: " << (bst.search(9) ? "存在" : "不存在") << endl; // 不存在

    // 删除示例
    bst.remove(3); // 删除有两个子节点的节点
    cout << "删除3后的中序遍历: ";
    bst.printInorder(); // 2 4 5 6 7 8 

    bst.remove(7); // 删除有两个子节点的节点
    cout << "删除7后的中序遍历: ";
    bst.printInorder(); // 2 4 5 6 8 

    bst.remove(2); // 删除叶子节点
    cout << "删除2后的中序遍历: ";
    bst.printInorder(); // 4 5 6 8 
    return 0;
}