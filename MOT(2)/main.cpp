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

Mat read_trans(string path);    //读取图片转换成灰度图

#define IM_NUM 100 //处理图片的张数

//权重超参数
#define W_Center 0.8
#define W_Area 0.2

#define PI 3.1415926

//全局变量
Mat pre,pre_gray,pre_dst;
Mat aft,aft_gray,aft_dst;
Mat trace_gray,trace_dst,trace_draw;
vector<RotatedRect> pre_minellipse;   //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果
vector<RotatedRect> aft_minellipse;
//vector<vector<RotatedRect>> minellipse(IM_NUM);      //minellipse[i]    存储第i幅图片的信息
vector<RotatedRect> trace_minellipse;     //存储第一幅图像的信息

//vector<int> matching;          //记录匹配结果，如果没有则为-1
vector<vector<int>> matching(IM_NUM-1);    //matching[i]  记录第i幅图片和它后面一张的匹配结果

int main()
{
	string s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	string num;    
	string s2 = ".jpg";
	trace_gray = read_trans("D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_0000.jpg");

	segmentation SEG;
	trace_draw = SEG.segment("trace", trace_gray, trace_dst, trace_minellipse);       //存储开始的信息
	pre_minellipse = trace_minellipse;      //pre_minellipse 为每次匹配中的 前帧

	
	int pre_num, aft_num;
	HungarianAlgorithm HungAlgo;
	Scalar color = Scalar(0, 255, 0);   //轨迹颜色
	//vector<double> temp;  //临时变量，初始化使用

	for (int i = 1; i < IM_NUM; i++)    //分割所有图片
	{
		if (i < 10) 
			s1 += "000";
		else if (i<100)
			s1 += "00";
		else
			s1 += "0";

		num = to_string(i);
		s1 += num;
		s1 += s2;               //s1  为组成的路径名
		aft_gray = read_trans(s1);
		SEG.segment("im", aft_gray, aft_dst, aft_minellipse);    // aft_minellipse 为每次匹配中的 后帧

		//初始化权值矩阵
		pre_num = pre_minellipse.size();
		aft_num = aft_minellipse.size();
		vector<double> temp;  //临时变量，初始化使用
		temp.resize(aft_num);
		vector<vector<double>> connect;    //权值矩阵
		connect.resize(pre_num, temp);
		for (int a = 0; a < pre_num; a++)    //初始化，要设计各种特征的权重超参数
		{
			for (int b = 0; b < aft_num; b++)
			{
				connect[a][b] = W_Center * sqrt(pow(pre_minellipse[a].center.y - aft_minellipse[b].center.y, 2) + pow(pre_minellipse[a].center.x - aft_minellipse[b].center.x, 2))
					//	+ W_Area * PI * abs((pre_minellipse[a].size.height / 2) *(pre_minellipse[a].size.width / 2) - (aft_minellipse[b].size.height / 2)*(aft_minellipse[b].size.height / 2) )
					;
			}
		}
	
		HungAlgo.Solve(connect, matching[i-1]);     //Hungarian 匹配
		for (int k = 0; k < matching[i-1].size(); k++)             //根据 matching[] 结果画线
		{
			//cout << matching[i-1][k] << endl;
			if (matching[i-1][k] != -1)
			{
				line(trace_draw, pre_minellipse[k].center, aft_minellipse[matching[i-1][k]].center, color, 1, 8);
			}
		}

		pre_minellipse = aft_minellipse;      //后帧信息赋值给前帧
		s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	}
	

	namedWindow("trace", WINDOW_AUTOSIZE);
	imshow("trace", trace_draw);
	waitKey(0);
	return 0;
}

Mat read_trans(string path)    //读取图片转换成灰度图
{
	Mat img = imread(path);
	if (img.empty())
	{
		//检查是否读取图像
		cout << "error! 图片读取失败\n";
		waitKey(0);
		getchar();
		return img;
	}
	else
	{
		cvtColor(img, img, COLOR_BGR2GRAY);   //转换为灰度图
		return img;
	}
}


