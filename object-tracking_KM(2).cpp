#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<opencv2/opencv.hpp>
#include<cmath>
#include<stdlib.h>

#include "Hungarian.h"

using namespace std;
using namespace cv;

//函数声明
Mat my_segmentation(string im_name, Mat &im_gray, Mat &im_dst, vector<vector<Point>> &contours, vector<RotatedRect> &im_minEllipse, vector<double> &im_roundness);  //分割函数
int KM_Hungarian();  //匈牙利算法，此处为带权重的二部图匹配
bool find(int i,int aft_num);
void set_val(vector<double> x,double val)
{
	vector<double>::iterator it;
	for (it = x.begin(); it != x.end(); it++)
		*it = val;
}
void set_b_val(vector<bool> x, bool val)
{
	vector<bool >::iterator it;
	for (it = x.begin(); it != x.end(); it++)
		*it = val;
}
void draw_tracklets();     //画出tracklets

int thr = 40;  //初始化阈值
#define IM_SIZE 5   //分割时，筛选尺寸
#define IM_ROUNDNESS 0.4   //分割时，筛选圆度

//权重超参数
#define W_Center 0.65
#define W_Area 0.3
#define W_Round 0.05

//全局变量
Mat pre, pre_gray, pre_dst;
Mat aft, aft_gray, aft_dst;
//contours一个双重向量（向量内每个元素保存了一组由连续的Point构成的点的集合的向量），每一组点集就是一个轮廓，有多少轮廓，contours就有多少元素
vector<vector<Point>> pre_contours;
vector<vector<Point>> aft_contours;
vector<RotatedRect> pre_minellipse;//RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果
vector<RotatedRect> aft_minellipse;
vector<double> pre_roundness;   //圆度
vector<double> aft_roundness;

//匈牙利算法所需数据结构
vector<vector<double>> connect;    //关系矩阵
vector<double> ex_pre, ex_aft;     //顶标
vector<bool> vis_pre, vis_aft;     //记录每一轮匹配过的节点
vector<int> matching;          //记录每一个aft的匹配结果，如果没有则为-1
vector<double> slack;     //松弛值
//KM算法可以优化到O(n3)
//一个优化是对YY顶点引入松弛函数slackslack，slack[j]slack[j]保存跟当前节点jj相连的节点ii的lx[i] + ly[j]−weight(i, j)lx[i] + ly[j]−weight(i, j)的最小值，于是求deltadelta时只需O(n)枚举不在交错树中的YY顶点的最小slackslack值即可。
//松弛值可以在匈牙利算法检查相等子树边失败时进行更新，同时在修改标号后也要更新，具体参考代码实现。
const double INF = 0x7FF0000000000000;

double matching_sum = 0;    //匹配成功的权值

int main()
{
	int M = 2;
	int N = 3;
	vector<vector<double>> cost(M,vector<double>(N));
	vector<int> assignment;
	double dist;
	for (int i=0; i < M; i++)
	{
		for (int j = 0; j < N; j++)
			cost[i][j] = rand()%10;
	}
	HungarianAlgorithm ASS;
	cout<<ASS.Solve(cost, assignment);

	cout << "ok";



	//pre = imread("D:\\pre.jpg");
 // 	if (pre.empty())
	//{
	//	//检查是否读取图像
	//	cout << "error! 图片读取失败\n";
	//	waitKey(0);
	//	getchar();
	//	return -1;
	//}
	//else
	//{
	//	cvtColor(pre, pre_gray, COLOR_BGR2GRAY);   //转换为灰度图
	//}
	//aft = imread("D:\\aft.jpg");
	//if (aft.empty())
	//{
	//	//检查是否读取图像
	//	cout << "error! 图片读取失败\n";
	//	waitKey(0);
	//	getchar();
	//	return -1;
	//}
	//else
	//{
	//	cvtColor(aft, aft_gray, COLOR_BGR2GRAY);   //转换为灰度图
	//}

	//pre_dst = my_segmentation("pre", pre_gray, pre_dst, pre_contours, pre_minellipse, pre_roundness);  //分割
	//aft_dst = my_segmentation("aft", aft_gray, aft_dst, aft_contours, aft_minellipse, aft_roundness);
	//
	//cout<<KM_Hungarian();    //匈牙利算法二部图匹配
	//cout << "匹配权值和：" << matching_sum;
	//draw_tracklets();
	//waitKey(0);
	return 0;
}


Mat my_segmentation(string im_name, Mat &im_gray, Mat &im_dst, vector<vector<Point>> &contours, vector<RotatedRect> &im_minEllipse, vector<double> &im_roundness)
{
	//hierarchy是一个向量，向量内每个元素都是一个包含4个int型的数组。向量hierarchy内的元素和轮廓向量contours内的元素是一一对应的，向量的容量相同。
	//hierarchy内每个元素的4个int型变量是hierarchy[i][0] ~hierarchy[i][3]，分别表
	//示当前轮廓 i 的后一个轮廓、前一个轮廓、父轮廓和内嵌轮廓的编号索引
	vector<Vec4i> hierarchy;

	cout << "##################  threshold:   " << thr << "    #########################" << endl;
	threshold(im_gray, im_dst, thr, 255, THRESH_BINARY);  //阈值函数，图像分割  THRESH_TOZERO\THRESH_BINARY\.....
	findContours(im_dst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));   //找轮廓,Point是偏移量

	//对轮廓进行一些预处理操作
	vector<vector<Point>>::iterator contours_it;     //轮廓指针
	for (contours_it = contours.begin(); contours_it != contours.end();)    ////删去小于10个像素的轮廓
	{
		if ((*contours_it).size() < 10)      //删去小于10个像素的边缘
		{
			contours_it = contours.erase(contours_it);    //若删除成功，则指针已经+1
		}
		else
		{
			++contours_it;
		}
	}

	im_roundness.resize(contours.size());    //  每个轮廓的圆形度
	im_minEllipse.resize(contours.size());    //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果

	for (int i = 0; i < contours.size(); i++)   //对每一个轮廓画最小包围圆
	{
		//对指定的点集进行多边形逼近的函数,连续光滑曲线折线化，对图像轮廓点进行多边形拟合。
		//approxPolyDP(contours[i], contours[i], 1, true);    //true代表曲线是闭合的
		if (contours[i].size() > 5)          //至少6个点画椭圆
		{
			im_minEllipse[i] = fitEllipse(contours[i]);    //最小外接椭圆
			im_roundness[i] = (4 * CV_PI * contourArea(contours[i])) / (arcLength(contours[i], true)  *arcLength(contours[i], true));//圆度
		}
		else
			cout << "像素点数量<5" << endl;
	}

	//..按条件筛选。。。。。。。。。。。。。。。。。。。。。。
	vector<double>::iterator roundness_it = im_roundness.begin();    //  圆形度指针
	vector<RotatedRect>::iterator minEllipse_it = im_minEllipse.begin();    //椭圆指针
	for (contours_it = contours.begin(); contours_it != contours.end();)    //it++要写到for里面，否则不能删除连续值
	{
		if ((*minEllipse_it).size.width < IM_SIZE || (*roundness_it) < IM_ROUNDNESS)
		{
			contours_it = contours.erase(contours_it);
			roundness_it = im_roundness.erase(roundness_it);
			minEllipse_it = im_minEllipse.erase(minEllipse_it);
		}
		else
		{
			++contours_it;
			++roundness_it;
			++minEllipse_it;
		}
	}

	//画图
	Mat drawing = Mat::zeros(im_dst.size(), CV_8UC3);   //重绘一个三通道的，背景为黑色的图
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color_R = Scalar(0, 0, 255);
		Scalar color = Scalar(255, 0, 0);
		drawContours(drawing, contours, i, color_R, 1, 8, vector<Vec4i>(), 0, Point());
		ellipse(drawing, im_minEllipse[i], color, 1, 8);
		cout << (i + 1) << im_minEllipse[i].center << "   minor axis:  " << setprecision(6) << im_minEllipse[i].size.width << "  long axis:  " << setprecision(6) << im_minEllipse[i].size.height << "  roundness:" << setprecision(6) << im_roundness[i] << endl;
	}
	cout << endl << endl;
	/// 在窗体中显示结果
	namedWindow(im_name, WINDOW_AUTOSIZE);
	imshow(im_name, drawing);
	return drawing;
}

bool find(int i,int aft_num)   //对pre中的每一个 i 进行查找
{
	//if (ex_pre[i]<0)
	//	return false;

	vis_pre[i] = true;

	for (int j = 0; j < aft_num; j++) 
	{
		if (vis_aft[j]) 
			continue; // 每一轮匹配 每个aft元素只尝试一次

		int gap = ex_pre[i] + ex_aft[j] - connect[i][j];

		if (gap == 0)    // 如果符合要求
		{  
			vis_aft[j] = true;
			if (matching[j] == -1 || find(matching[j],aft_num))    // 找到一个没有匹配的男生 或者该男生的妹子可以找到其他人
			{    
				matching[j] = i;
				return true;
			}
		}
		else 
		{
			if (slack[j] > gap) // slack 可以理解为该aft要想能够有匹配 还需多少期望值 取最小值
				slack[j] = gap;
		}
	}

	return false;
}

int KM_Hungarian()
{
	//初始化：改变尺寸，初值赋 0
	int pre_num = pre_minellipse.size();
	int aft_num = aft_minellipse.size();
	slack.resize(aft_num);
	ex_pre.resize(pre_num);
	ex_aft.resize(aft_num);
	vis_pre.resize(pre_num);
	vis_aft.resize(aft_num);
	matching.resize(aft_num);
	
	//用 NO_MATCHING 初始化 matching,未匹配为-1
	vector<int>::iterator it;
	for (it = matching.begin(); it != matching.end(); it++)
		*it = -1;
	
	//关系矩阵 connect[][] 初始化
	vector<double> temp(aft_num);  //临时变量，初始化使用
	set_val(temp, 0);
	connect.resize(pre_num, temp);
	for (int a = 0; a < pre_num; a++)    //初始化，要设计各种特征的权重超参数
	{
		for (int b = 0; b < aft_num; b++)
		{
			connect[a][b] = sqrt( pow(pre_minellipse[a].center.y - aft_minellipse[b].center.y, 2) + pow(pre_minellipse[a].center.x - aft_minellipse[b].center.x, 2) );
			//connect[a][b] = 2.123 ;
		}
	}
	//显示connect矩阵
	cout << endl << endl << "#################################" << endl;
	for (int a = 0; a < pre_num; a++)
	{
		for (int b = 0; b < aft_num; b++)
		{
			cout << connect[a][b]<<" ";
		}
		cout << endl;
	}


	
	//初始化aft顶标,全为0
	set_val(ex_aft, 0);

	//初始化pre顶标,为相连的边中最大的
	for (int i = 0; i < pre_num; ++i) 
	{
		ex_pre[i] = connect[i][0];
		for (int j = 1; j < pre_num; ++j) 
		{
			if (ex_pre[i] < connect[i][j])
				ex_pre[i] = connect[i][j];
		}
	}

	// 尝试为每一个pre进行匹配
	for (int i = 0; i < pre_num; ++i) 
	{
		set_val(slack, INF);   // 因为要取最小值 初始化为无穷大

		while (true) 
		{
			// 为每个pre匹配的方法是 ：如果找不到就降低期顶标，直到找到为止

			// 记录每轮匹配中pre,aft是否被尝试匹配过
			set_b_val(vis_pre, false);
			set_b_val(vis_aft, false);

			if (find(i,aft_num)) break;  // 找到归宿 退出

			// 如果不能找到 就降低期望值
			// 最小可降低的期望值delta
			int delta = INF;   //初始为无穷大
			for (int j = 0; j < aft_num; ++j)
			{
				if (!vis_aft[j] && delta > slack[j])
					delta = slack[j];                   //取没有访问过的aft中，为了能够得到匹配所需要的最小期望值
			}
			for (int i = 0; i < pre_num; ++i) 
			{
				// 匹配失败，所有访问过的pre降低期望值delta
				if (vis_pre[i]) 
					ex_pre[i] -= delta;
			}
			for (int j = 0; j < aft_num; j++)
			{
				// 所有访问过的aft增加期望值
				if (vis_aft[j]) 
					ex_aft[j] += delta;
				// 没有访问过的aft 因为pre们的期望值降低，距离得到匹配期望又进了一步！
				else 
					slack[j] -= delta;
			}

		}
	}

	// 匹配完成 求出所有配对的权值的和
	matching_sum = 0;
	for (int j = 0; j < aft_num; j++)
		if (matching[j]!=-1)
			matching_sum += connect[matching[j]][j];

	return matching_sum;

}

void draw_tracklets()     //画出tracklets
{
	Scalar color = Scalar(0, 255, 0);
	for (int i = 0; i< matching.size(); i++)
	{
		cout << matching[i] << endl;
		if (matching[i] != -1)
		{
			line(pre_dst, pre_minellipse[matching[i]].center, aft_minellipse[i].center, color, 1, 8);
		}
	}
	imshow("pre", pre_dst);
}

