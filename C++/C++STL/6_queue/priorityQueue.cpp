#include<iostream>
#include<queue>
using namespace std;

int main(){

	priority_queue<int, deque<int>, greater<int>> pqA;  //使用deque 值越小，优先级越大
    pqA.push(1);
	pqA.push(2);
	pqA.push(3);
	pqA.push(3);
	pqA.push(4);
	pqA.push(5);
	pqA.push(3);

    while(!pqA.empty()){
            cout<<pqA.top()<<" ";//读取队首的元素,但元素不出列
            pqA.pop();           //出队列
    }

	cout<<endl;

    return 0;
}