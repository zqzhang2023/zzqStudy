#include<iostream>
#include<queue>
#include<algorithm>
#include<vector>
using namespace std;

class GraphMatrix{
private:
    int nodeNum = 5;
    int data[5][5];
public:
    GraphMatrix(){
        for(int i=0;i<nodeNum;i++){
            for(int j=0;j<nodeNum;j++){
                data[i][j] = 0;
            }
        }
        data[0][1] = 1;data[0][3] = 1;
        data[1][3] = 1;data[1][2] = 1;
        data[2][4] = 1;
        data[3][2] = 1;data[3][4] = 1;
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

    int indegree(int x){
        if(x>nodeNum-1||x<0){
            return -1;
        }
        int result = 0;
        for(int i = 0; i < nodeNum; i++) {
            if(data[i][x] != 0) { // 计算入度
                result++;
            }
        }
        return result;
    }

};


vector<int> topologicalSort(GraphMatrix &graph) {
    const int nodeNum = 5; // 节点数固定为5
    vector<int> inDegree(nodeNum);
    for (int i = 0; i < nodeNum; ++i) {
        inDegree[i] = graph.indegree(i);
    }

    queue<int> q;
    for (int i = 0; i < nodeNum; ++i) {
        if (inDegree[i] == 0) {
            q.push(i);
        }
    }

    vector<int> result;
    while (!q.empty()){
        int u = q.front();
        q.pop();
        result.push_back(u);
        for(int v = graph.firstNeighbor(u);v!=-1;v = graph.nextNeighbor(u, v)){
            if (--inDegree[v] == 0) {
                q.push(v);
            }
        }
    }

    if (result.size() != nodeNum) {
        return {}; // 存在环
    }
    return result;
}


int main() {
    GraphMatrix graph;
    vector<int> sorted = topologicalSort(graph);

    if (sorted.empty()) {
        cout << "图中存在环，无法进行拓扑排序" << endl;
    } else {
        cout << "拓扑排序结果：";
        for (int node : sorted) {
            cout << node << " ";
        }
        cout << endl;
    }

    return 0;
}