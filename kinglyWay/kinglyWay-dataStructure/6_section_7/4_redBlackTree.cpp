#include <iostream>
using namespace std;

// 颜色枚举
enum Color { RED, BLACK };

// 红黑树节点结构
struct RBNode {
    int val;
    Color color;
    RBNode *left, *right, *parent;
    RBNode(int x) : val(x), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}
};

class RedBlackTree {
private:
    RBNode* root;
    RBNode* nil; // 哨兵节点（表示空叶子节点）

    // 左旋操作
    void leftRotate(RBNode* x) {
        RBNode* y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }

    // 右旋操作
    void rightRotate(RBNode* x) {
        RBNode* y = x->left;
        x->left = y->right;
        if (y->right != nil) {
            y->right->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->right) {
            x->parent->right = y;
        } else {
            x->parent->left = y;
        }
        y->right = x;
        x->parent = y;
    }

    // 插入修复（处理颜色冲突）
    void insertFixup(RBNode* z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                RBNode* y = z->parent->parent->right; // 叔叔节点
                if (y->color == RED) {
                    // Case 1: 叔叔是红色
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        // Case 2: 叔叔是黑色且z是右孩子
                        z = z->parent;
                        leftRotate(z);
                    }
                    // Case 3: 叔叔是黑色且z是左孩子
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else { // 对称操作（父节点是右孩子）
                RBNode* y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK; // 根节点始终黑色
    }

    // 删除修复（处理颜色失衡）
    void deleteFixup(RBNode* x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                RBNode* w = x->parent->right; // 兄弟节点
                if (w->color == RED) {
                    // Case 1: 兄弟是红色
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    // Case 2: 兄弟的子节点都是黑色
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        // Case 3: 兄弟的右孩子是黑色
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    // Case 4: 兄弟的右孩子是红色
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root; // 终止循环
                }
            } else { // 对称操作（x是右孩子）
                RBNode* w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }

    // 查找最小节点
    RBNode* minimum(RBNode* node) {
        while (node->left != nil) node = node->left;
        return node;
    }

    // 中序遍历辅助函数
    void inorderTraversal(RBNode* node) {
        if (node == nil) return;
        inorderTraversal(node->left);
        cout << node->val << (node->color == RED ? "(R) " : "(B) ");
        inorderTraversal(node->right);
    }

public:
    RedBlackTree() {
        nil = new RBNode(-1);
        nil->color = BLACK;
        root = nil;
    }

    // 插入接口
    void insert(int val) {
        RBNode* z = new RBNode(val);
        RBNode* y = nil;
        RBNode* x = root;
        while (x != nil) {
            y = x;
            if (z->val < x->val) {
                x = x->left;
            } else {
                x = x->right;
            }
        }
        z->parent = y;
        if (y == nil) {
            root = z;
        } else if (z->val < y->val) {
            y->left = z;
        } else {
            y->right = z;
        }
        z->left = nil;
        z->right = nil;
        insertFixup(z);
    }

    // 删除接口
    void remove(int val) {
        RBNode* z = root;
        while (z != nil) {
            if (z->val == val) break;
            else if (val < z->val) z = z->left;
            else z = z->right;
        }
        if (z == nil) return; // 未找到

        RBNode* y = z;
        Color yOriginalColor = y->color;
        RBNode* x;

        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            yOriginalColor = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        delete z;
        if (yOriginalColor == BLACK) {
            deleteFixup(x);
        }
    }

    // 替换节点辅助函数
    void transplant(RBNode* u, RBNode* v) {
        if (u->parent == nil) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // 查找接口
    bool search(int val) {
        RBNode* curr = root;
        while (curr != nil) {
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

int main() {
    RedBlackTree rbt;

    // 插入测试
    rbt.insert(10);
    rbt.insert(20);
    rbt.insert(30);
    rbt.insert(40);
    rbt.insert(50);
    rbt.insert(25);

    cout << "插入后的中序遍历: ";
    rbt.printInorder(); // 输出示例：10(B) 20(R) 25(R) 30(B) 40(R) 50(R)

    // 查找测试
    cout << "查找25: " << (rbt.search(25) ? "存在" : "不存在") << endl; // 存在
    cout << "查找100: " << (rbt.search(100) ? "存在" : "不存在") << endl; // 不存在

    // 删除测试
    rbt.remove(30); // 删除根节点
    cout << "删除30后的中序遍历: ";
    rbt.printInorder(); // 示例输出：10(B) 20(R) 25(B) 40(R) 50(R)

    rbt.remove(25); // 删除叶子节点
    cout << "删除25后的中序遍历: ";
    rbt.printInorder(); // 示例输出：10(B) 20(B) 40(R) 50(R)

    return 0;
}