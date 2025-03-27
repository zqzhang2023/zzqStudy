#include<iostream>
#include<set>
using namespace std;

int main(){

    set<int>  setInt;
    setInt.insert(1);
    setInt.insert(2);
    setInt.insert(3);
    setInt.insert(4);
    setInt.insert(5);

    set<int>::iterator it1 = setInt.find(4);
    cout<<*it1<<endl;

    int iCount = setInt.count(3);
    cout<<iCount<<endl;
    iCount = setInt.count(100);	
    cout<<iCount<<endl;


    set<int>::iterator it2 = setInt.lower_bound(3);
    set<int>::iterator it3 = setInt.upper_bound(3);
    cout<<*it2<<"  "<<*it3<<endl;

    pair< set<int>::iterator, set<int>::iterator > pairIt = setInt.equal_range(4);  
    cout<<*pairIt.first<<"  "<<*pairIt.second<<endl;

    return 0;
}