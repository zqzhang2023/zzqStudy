#include<iostream>
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
        if(x>nodeNum-1||x<0){
            return -1;
        }
        if(y>nodeNum-1||y<0){
            return -1;
        }

        for (int  i = y+1; i < nodeNum; i++){
            if(data[x][i]!=0){
                return i;
            }
        }
        return -1;   
    }
};

class GraphTable{
private:
    const int nodeNum = 5;
    vector<int> data[5];
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

    return 0;
}