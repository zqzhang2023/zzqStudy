#include<iostream>
#include<vector>
using namespace std;


void merge(vector<int>& nums1, int m, vector<int>& nums2, int n) {
    if(n==0){
        return;
    }
    if(m==0){
        for(int k=0;k<n;k++){
            nums1[k] = nums2[k];
        }            // 找到比较小的元素，并处理
        return;
    }

    vector<int> resultList;
    int i=0,j=0;
    while(i<m||j<n){
        if(i==m){
            for(j;j<n;j++){
                resultList.push_back(nums2[j]);
            } 
            break;
        }

        if(j==n){
            for(i;i<m;i++){
                resultList.push_back(nums1[i]);
            } 
            break;
        }

        if(nums1[i]<=nums2[j]&&i<m){
            resultList.push_back(nums1[i]);
            i++;
            continue;
        }

        if(nums1[i]>nums2[j]&&j<n){
            resultList.push_back(nums2[j]);
            j++;
            continue;
        }
    }

    for(int k=0;k<m+n;k++){
        nums1[k] = resultList[k];
    }
    
}


int main(){
    vector<int> nums1 = {1,0};
    int m = 1;
    vector<int> nums2 = {2};
    int n = 1;
    merge(nums1,m,nums2,n);

    for (int i = 0; i < m+1; i++){
        cout<<nums1[i]<<endl;
    }

    return 0;

}