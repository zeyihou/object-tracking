#include<iostream>
#include<queue>
#include<vector>
#include<cmath>
#include<algorithm>
using namespace std;
const int N = 2;

int girl[N] = {100,100};   //足够大的一个初始值，标志没有分配过
int used[N] = {0,0};
int m=N, n=N;
int line[N][N] = { { 1, 1 }, { 1, 0 } };
int all;
bool find(int x){
	int i, j;
	for (j = 0; j <= n; j++){    //扫描每个目标  
		if (line[x][j] == 1 && used[j] == 0)
			//如果有连线并且还没有标记过(这里标记的意思是这次查找曾试图改变过该目标的归属问题，但是没有成功，所以就不用瞎费工夫了） 
			//used[]表示每轮中，aft中的每一个元素，只能匹配一次
		{
			used[j] = 1;
			if (girl[j] == 100 || find(girl[j])) {
				//名花无主或者能腾出个位置来，这里使用递归  
				girl[j] = x;
				return true;
			}
		}
	}
	return false;
}
int main()
{
	for (int k = 0; k <= m; k++)
	{
		memset(used, 0, sizeof(used));    //这个在每一步中清空  
		if (find(k))
			all += 1;
	}
	cout << all << endl;
	cout << girl[0] << endl;
	cout << girl[1] << endl;
	getchar();
	return 0;
}
