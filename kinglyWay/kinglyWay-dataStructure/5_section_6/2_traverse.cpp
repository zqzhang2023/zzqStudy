#include<iostream>
#include<queue>
using namespace std;

class GraphMatrix{
private:
    int nodeNum = 5;
    int data[5][5];
    bool visited[5];
public:
    GraphMatrix(){
        for(int i=0;i<nodeNum;i++){
            for(int j=0;j<nodeNum;j++){
                data[i][j] = 0;
            }
        }
        data[0][1] = 1;   // 0 -> 1
        data[0][3] = 1;   // 0 -> 3
        data[1][4] = 1;   // 1 -> 4
        data[2][4] = 1;   // 2 -> 4
        data[3][1] = 1;   // 3 -> 1
        data[4][3] = 1;   // 4 -> 3
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

    //广度优先遍历BFS Traverse
    void BFSTraverse(){
        //初始化 visited
        for(int i=0;i<nodeNum;i++){
            visited[i] = false;
        };
        //遍历 为了防止这个图是非连通图，所以必须这么写
        for(int i=0;i<nodeNum;i++){
            if (!visited[i]) {
                BFS(i);
            }
            
        };
    }

    void BFS(int node){
        queue<int> bfsQueue;
        bfsQueue.push(node);
        cout<<node<<" ";
        visited[node] = true;
        while(!bfsQueue.empty()){
            int temp = bfsQueue.front();
            bfsQueue.pop();
            for (int i = firstNeighbor(temp); i != -1; i = nextNeighbor(temp,i)) {
                if (!visited[i]){
                    bfsQueue.push(i);
                    cout<<i<<" ";
                    visited[i] = true;
                }
                
            }
            
        }
    }
    //深度优先搜索DFS Traverse
    void DFSTraverse(){
        //初始化 visited
        for(int i=0;i<nodeNum;i++){
            visited[i] = false;
        };
        //遍历 为了防止这个图是非连通图，所以必须这么写
        for(int i=0;i<nodeNum;i++){
            if (!visited[i]) {
                DFS(i);
            }
            
        };
    }

    void DFS(int node){
        cout<<node<<" ";
        visited[node]=true;
        for (int i = firstNeighbor(node); i != -1; i = nextNeighbor(node,i)){
            if (!visited[i]){
                DFS(i);
            }
            
        }
        
    }

};


class GraphTable{
private:
    const int nodeNum = 5;
    vector<int> data[5];
    bool visited[5];
public:
    GraphTable(){
        data[0].push_back(1);   // 0 -> 1
        data[0].push_back(3);   // 0 -> 3
        data[1].push_back(4);   // 1 -> 4
        data[2].push_back(4);   // 2 -> 4
        data[3].push_back(1);   // 3 -> 1
        data[4].push_back(3);   // 4 -> 3
    }

    int firstNeighbor(int x) {
        if (x < 0 || x >= nodeNum || data[x].empty()) {
            return -1;
        }
        return data[x][0];
    }

    int nextNeighbor(int x, int y) {
        if (x < 0 || x >= nodeNum || y < 0 || y >= nodeNum) {
            return -1;
        }
        for (int i = 0; i < data[x].size(); ++i) {
            if (data[x][i] == y) {
                if (i + 1 < data[x].size()) {
                    return data[x][i + 1];
                } else {
                    return -1;
                }
            }
        }
        return -1;
    }


        //广度优先遍历BFS Traverse
    void BFSTraverse(){
        //初始化 visited
        for(int i=0;i<nodeNum;i++){
            visited[i] = false;
        };
        //遍历 为了防止这个图是非连通图，所以必须这么写
        for(int i=0;i<nodeNum;i++){
            if (!visited[i]) {
                BFS(i);
            }
            
        };
    }

    void BFS(int node){
        queue<int> bfsQueue;
        bfsQueue.push(node);
        cout<<node<<" ";
        visited[node] = true;
        while(!bfsQueue.empty()){
            int temp = bfsQueue.front();
            bfsQueue.pop();
            for (int i = firstNeighbor(temp); i != -1; i = nextNeighbor(temp,i)) {
                if (!visited[i]){
                    bfsQueue.push(i);
                    cout<<i<<" ";
                    visited[i] = true;
                }
                
            }
            
        }
    }
    //深度优先搜索DFS Traverse
    void DFSTraverse(){
        //初始化 visited
        for(int i=0;i<nodeNum;i++){
            visited[i] = false;
        };
        //遍历 为了防止这个图是非连通图，所以必须这么写
        for(int i=0;i<nodeNum;i++){
            if (!visited[i]) {
                DFS(i);
            }
            
        };
    }

    void DFS(int node){
        cout<<node<<" ";
        visited[node]=true;
        for (int i = firstNeighbor(node); i != -1; i = nextNeighbor(node,i)){
            if (!visited[i]){
                DFS(i);
            }
            
        }
        
    }
};

int main(){

     // 测试 GraphMatrix
     GraphMatrix gm;
     cout << "Testing GraphMatrix:" << endl;
     for (int x = 0; x < 5; x++) {
         cout << "Node " << x << " neighbors: ";
         int current = gm.firstNeighbor(x);
         if (current == -1) {
             cout << "None" << endl;
             continue;
         }
         cout << current;
         while ((current = gm.nextNeighbor(x, current)) != -1) {
             cout << ", " << current;
         }
         cout << endl;
     }
 
     // 测试 GraphMatrix 的 BFS 和 DFS
     cout << "\nTesting GraphMatrix BFS and DFS:" << endl;
     cout << "BFS starting from 0: ";
     gm.BFSTraverse();
     cout<<endl;
     cout << "DFS starting from 0: ";
     gm.DFSTraverse();
     cout<<endl;
 
     // 测试 GraphTable
     GraphTable gt;
     cout << "\nTesting GraphTable:" << endl;
     for (int x = 0; x < 5; x++) {
         cout << "Node " << x << " neighbors: ";
         int current = gt.firstNeighbor(x);
         if (current == -1) {
             cout << "None" << endl;
             continue;
         }
         cout << current;
         while ((current = gt.nextNeighbor(x, current)) != -1) {
             cout << ", " << current;
         }
         cout << endl;
     }
 
     // 测试 GraphTable 的 BFS 和 DFS
     cout << "\nTesting GraphTable BFS and DFS:" << endl;
     cout << "BFS starting from 0: ";
     gt.BFSTraverse();
     cout<<endl;
     cout << "DFS starting from 0: ";
     gt.DFSTraverse();
     cout<<endl;
 
    

    return 0;
}