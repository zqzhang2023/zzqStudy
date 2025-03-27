#include <iostream>
using namespace std;
 
class Father {
public:
    virtual void play() {
        cout << "到 KTV 唱歌..." << endl;
    }
};
 
class Son :public Father {
public:
    virtual void play() {
        cout << "一起打王者吧！" << endl;
    }
};
 
void party(Father **men, int n) {
    for (int i = 0; i<n; i++) {
        men[i]->play();
    }
}
 
int main(void) {
    Father father;
    Son son1, son2;
    Father* men[] = { &father, &son1, &son2 };
    party(men, sizeof(men) / sizeof(men[0]));
 
    return 0;
}