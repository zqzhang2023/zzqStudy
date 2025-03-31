#include<iostream>
#include<vector>
#include<algorithm>
using namespace std;

class GraphMatrix{
private:
    int nodeNum = 6;
    int data[6][6];
public:
    GraphMatrix(){
        for(int i=0;i<nodeNum;i++){
            for(int j=0;j<nodeNum;j++){
                data[i][j] = 0;
            }
        }
        data[0][1] = 6;data[0][2] = 1;data[0][3] = 5;
        data[1][0] = 6;data[1][2] = 5;data[1][4] = 3;
        data[2][0] = 1;data[2][1] = 5;data[2][4] = 6;data[2][5] = 4;data[2][3] = 5;
        data[3][0] = 5;data[3][2] = 5;data[3][5] = 2;
        data[4][1] = 3;data[4][2] = 6;data[4][5] = 6;
        data[5][4] = 6;data[5][2] = 4;data[5][3] = 2;
    }

    int firstNeighbor(int x){
        if(x>nodeNum-1||x<0){
            return -1;
        }

        for (int  i = 0; i < nodeNum; i++){
            if(data[x][i]!=0){
                return i;
            }
        }

        return -1;
        
    }

    int nextNeighbor(int x,int y){
        if(x>nodeNum-1||x<0||y>nodeNum-1||y<0){
            return -1;
        }
        for (int  i = y+1; i < nodeNum; i++){
            if(data[x][i]!=0){
                return i;
            }
        }
        return -1;   
    }

    int getWeight(int x,int y){
        if(x>nodeNum-1||x<0||y>nodeNum-1||y<0){
            return -1;
        }
        return data[x][y];
    }

};


void prim(GraphMatrix &graph) {
    const int n = 6;
    int parent[n];   // 存储MST中节点的父节点
    int key[n];      // 存储各节点到MST的最小权重
    bool mstSet[n];  // 记录节点是否在MST中


    // 初始化
    for(int i=0; i<n; i++) {
        key[i] = 9999;
        mstSet[i] = false;
    }

    key[0] = 0;      // 选择节点0作为起始点
    parent[0] = -1;  // 起始点没有父节点

    // 构建MST需要n-1次迭代 有n个点就得有n-1个边
    for(int count=0; count < n-1; count++) {
        // 找出当前未加入MST且key最小的节点
        int u = -1;
        int minKey = 9999;
        for(int i=0; i<n; i++) {
            if(!mstSet[i] && key[i] < minKey) {
                minKey = key[i];
                u = i;
            }
        }

        if(u == -1) break;  // 图不连通，无法形成MST

        mstSet[u] = true;  // 将节点u加入MST

        // 更新u的邻接节点的key值和父节点
        for(int v = graph.firstNeighbor(u);v!=-1;v = graph.nextNeighbor(u, v)){
            int weight = graph.getWeight(u, v);
            if(!mstSet[v] && weight < key[v]) {
                parent[v] = u;
                key[v] = weight;
            }
        }
    }


    // 输出MST
    cout << "Prim's MST:" << endl;
    int totalWeight = 0;
    for(int i=1; i<n; i++) {
        int w = graph.getWeight(parent[i], i);
        cout << parent[i] << " - " << i << " \tWeight: " << w << endl;
        totalWeight += w;
    }
    cout << "Total Weight: " << totalWeight << endl << endl;
}


// Kruskal算法所需的数据结构
struct Edge {
    int u, v, weight;
    Edge(int u, int v, int w) : u(u), v(v), weight(w) {}
    bool operator<(const Edge &other) const {
        return weight < other.weight;
    }
};

// 并查集实现
class UnionFind {
private:
    vector<int> parent;
public:
    UnionFind(int size) {
        parent.resize(size);
        for(int i=0; i<size; i++) parent[i] = i;
    }

    int find(int x) {
        if(parent[x] != x)
            parent[x] = find(parent[x]);  // 路径压缩
        return parent[x];
    }

    void unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        if(rootX != rootY)
            parent[rootX] = rootY;
    }
};


// Kruskal算法实现
void kruskal(GraphMatrix &graph) {
    vector<Edge> edges;
    // 收集所有无向边（只保存u < v的情况避免重复）
    for(int u=0; u<6; u++) {
        int v = graph.firstNeighbor(u);
        while(v != -1) {
            if(u < v) {  // 避免重复添加边
                edges.emplace_back(u, v, graph.getWeight(u, v));
            }
            v = graph.nextNeighbor(u, v);
        }
    }

    sort(edges.begin(), edges.end());

    UnionFind uf(6);
    vector<Edge> mst;
    int totalWeight = 0;

    // 遍历所有边构建MST
    for(Edge &e : edges) {
        if(uf.find(e.u) != uf.find(e.v)) {
            uf.unite(e.u, e.v);
            mst.push_back(e);
            totalWeight += e.weight;
            if(mst.size() == 5) break;  // MST有n-1条边
        }
    }

    // 输出结果
    cout << "Kruskal's MST:" << endl;
    for(Edge &e : mst) {
        cout << e.u << " - " << e.v << " \tWeight: " << e.weight << endl;
    }
    cout << "Total Weight: " << totalWeight << endl;
}


int main(){
    GraphMatrix graph;
    prim(graph);
    kruskal(graph);
    return 0;
}