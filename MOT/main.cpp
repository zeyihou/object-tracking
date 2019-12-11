#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<opencv2/opencv.hpp>
#include<cmath>
#include<stdlib.h>

#include "segmentation.h"
#include "Hungarian.h"

using namespace std;
using namespace cv;

void draw_tracklets(Mat draw);     //画出tracklets

#define IM_NUM 100  //处理图片的张数

//权重超参数
#define W_Center 0.8
#define W_Area 0.2

#define PI 3.1415926

//全局变量
Mat pre,pre_gray,pre_dst;
Mat aft,aft_gray,aft_dst;
vector<RotatedRect> pre_minellipse;   //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果
vector<RotatedRect> aft_minellipse;

//匈牙利算法所需数据结构
vector<vector<double>> connect;    //权值矩阵
vector<int> matching;          //记录每一个aft的匹配结果，如果没有则为-1

int main()
{

	pre = imread("D:\\Mot VS2013 project\\Mot test1\\video_dataset\\pre.jpg");
	 	if (pre.empty())
	{
		//检查是否读取图像
		cout << "error! 图片读取失败\n";
		waitKey(0);
		getchar();
		return -1;
	}
	else
	{
		cvtColor(pre, pre_gray, COLOR_BGR2GRAY);   //转换为灰度图
	}
	aft = imread("D:\\Mot VS2013 project\\Mot test1\\video_dataset\\aft.jpg");
	if (aft.empty())
	{
		//检查是否读取图像
		cout << "error! 图片读取失败\n";
		waitKey(0);
		getchar();
		return -1;
	}
	else
	{
		cvtColor(aft, aft_gray, COLOR_BGR2GRAY);   //转换为灰度图
	}

	segmentation SEG;
	Mat pre_draw = SEG.segment("pre_segment", pre_gray, pre_dst, pre_minellipse);
    Mat Pre_draw = SEG.segment("aft_segment", aft_gray, aft_dst, aft_minellipse);

	int pre_num = pre_minellipse.size();
	int aft_num = aft_minellipse.size();
	
	//关系矩阵 connect[][] 初始化
	vector<double> temp(aft_num);  //临时变量，初始化使用
	connect.resize(pre_num, temp);
	for (int a = 0; a < pre_num; a++)    //初始化，要设计各种特征的权重超参数
	{
		for (int b = 0; b < aft_num; b++)
		{
			connect[a][b] =W_Center * sqrt(pow(pre_minellipse[a].center.y - aft_minellipse[b].center.y, 2) + pow(pre_minellipse[a].center.x - aft_minellipse[b].center.x, 2))
			//	+ W_Area * PI * abs((pre_minellipse[a].size.height / 2) *(pre_minellipse[a].size.width / 2) - (aft_minellipse[b].size.height / 2)*(aft_minellipse[b].size.height / 2) )
				;
		}
	}
	//显示connect矩阵
	cout << endl << endl << "#################################" << endl;
	for (int a = 0; a < pre_num; a++)
	{
		for (int b = 0; b < aft_num; b++)
		{
			cout << connect[a][b] << " ";
		}
		cout << endl;
	}

	HungarianAlgorithm HungAlgo;
	cout << "总代价" << HungAlgo.Solve(connect, matching);
	cout<<"匹配个数"<<matching.size();

	draw_tracklets(pre_draw);
	waitKey(0);
	return 0;
}

void draw_tracklets(Mat draw)     //画出tracklets
{
	Scalar color = Scalar(0, 255, 0);
	for (int i = 0; i< matching.size(); i++)
	{
		cout << matching[i] << endl;
		if (matching[i] != -1)
		{
			line(draw, pre_minellipse[i].center, aft_minellipse[matching[i]].center, color, 1, 8);
		}
	}
	imshow("pre_dst", draw);
}

