#include<iostream>
#include<vector>
#include<algorithm>
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
        data[0][1] = 10;data[0][4] = 5;
        data[1][4] = 2;data[1][2] = 1;
        data[2][3] = 4;
        data[3][2] = 6;data[3][0] = 7;
        data[4][1] = 3;data[4][2] = 9;data[4][3] = 2;
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


void dijkstra(GraphMatrix &graph) {
    const int nodeNum = 5;
    const int INF = 999999;
    int dist[nodeNum];
    bool visited[nodeNum] = {false};

    for (int i = 0; i < nodeNum; i++){
        dist[i] = INF;
    }
    dist[0] = 0;

    for (int count = 0; count < nodeNum; count++) {
        int u = -1, minDist = INF;
        for (int i = 0; i < nodeNum; i++) {
            if (!visited[i] && dist[i] < minDist) {
                minDist = dist[i];
                u = i;
            }
        }
        if (u == -1) break;

        visited[u] = true;

        for(int v = graph.firstNeighbor(u);v!=-1;v = graph.nextNeighbor(u, v)){
            int weight = graph.getWeight(u, v);
            if (!visited[v] && dist[u] != INF && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
            }
        }

    }

    cout << "Dijkstra算法（起点0）的最短路径：" << endl;
    for (int i = 0; i < nodeNum; i++) {
        if (dist[i] != INF)
            cout << "0->" << i << " 的最短距离: " << dist[i] << endl;
        else
            cout << "0->" << i << " 不可达" << endl;
    }
}

void floyd(GraphMatrix &graph) {
    const int nodeNum = 5;
    const int INF = 999999;
    int dist[nodeNum][nodeNum];

    //设置一下，有边则距离设置为边的权重，反之，设置为无穷大
    for (int i = 0; i < nodeNum; i++) {
        for (int j = 0; j < nodeNum; j++) {
            if (i == j) dist[i][j] = 0;
            else {
                int w = graph.getWeight(i, j);
                dist[i][j] = (w != 0) ? w : INF;
            }
        }
    }

    for (int k = 0; k < nodeNum; k++){
        for (int i = 0; i < nodeNum; i++){
            for (int j = 0; j < nodeNum; j++){
                if (dist[i][k]!=INF&&dist[k][j]!=INF&&dist[i][j]>dist[i][k]+dist[k][j]) {
                    dist[i][j]=dist[i][k]+dist[k][j];
                }
                
            } 
        }
    }


    cout << "\nFloyd算法的所有顶点对最短路径：" << endl;
    for (int i = 0; i < nodeNum; i++) {
        for (int j = 0; j < nodeNum; j++) {
            if (dist[i][j] == INF) cout << "INF\t";
            else cout << dist[i][j] << "\t";
        }
        cout << endl;
    }
    
}

int main(){
    GraphMatrix graph;
    dijkstra(graph);
    floyd(graph);
    return 0;
}