#include<iostream>
#include<string>
#include<vector>
using namespace std;

int main()
{

vector<int> v(10);
for(int i=0;i<10;i++)
	v.push_back(i);

cout << v.size()<<"\n";
}

