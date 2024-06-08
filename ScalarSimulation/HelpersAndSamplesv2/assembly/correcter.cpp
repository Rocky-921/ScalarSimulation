#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(){
    ifstream filein("outIcache.txt");
    ofstream fileout("outcache.txt");
    vector<int> dat(512,0);
    int a,b,i=0;
    while(filein >> hex >> a) dat[i++]=a;
    for(int i=0;i<512;i+=2) fileout << hex << dat[i] << dat[i+1] << "\n";
}