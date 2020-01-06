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

Mat read_trans(string path);    //读取图片+转换成灰度图
void form_tracklets();          //第一轮逐帧匹配,形成reliable tracklets
void association();           //数据关联（对tracklets匈牙利算法匹配）

#define IM_NUM 50 //处理图片的张数
#define REMOTE 20   //相邻帧之前匹配的距离上限，距离超过REMOTE，则进行轨迹切断
#define TRACKLET_REMOTE 100    //相邻tracklet之前匹配的距离上限

//权重超参数
#define W_Euclidean 1

#define PI 3.1415926

//全局变量
Mat pre,pre_gray,pre_dst;
Mat aft,aft_gray,aft_dst;
Mat trace_gray,trace_dst,trace_draw;
vector<RotatedRect> pre_minellipse;   //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果
vector<RotatedRect> aft_minellipse;

struct obj
{
	vector<RotatedRect> obj_minellipse;      //obj_minellipse   存储第i幅图片的信息
	vector<int> pre_link;    //用来标记每一个目标是否可作为轨迹段起点
};
vector<obj> minellipse(IM_NUM);    //存储所有帧信息

vector<RotatedRect> trace_minellipse;     //存储第一幅图像的信息

//vector<int> matching;          //记录匹配结果，如果没有则为-1
vector<vector<int>> matching(IM_NUM-1);    //matching[i]  记录第i幅图片和它后面一张的匹配结果
//vector<vector<int>> origin(IM_NUM-1);       //每个轨迹段的开始点标志

struct tracklet      //轨迹段
{
	int t_s;   //开始帧   matching的下标
	int s_ID;   //tracklets起点在开始帧中的ID
	int t_e;   //结束帧
	int e_ID;   //tracklets终点在结束帧中的ID
	double len = 0;  //轨迹长度
	double v = 0;   //运动速度
};
vector<vector<tracklet>> tracklets_formulate(IM_NUM);   //存储第一轮逐帧匹配后的tracklets，tracklets[0]代表第一帧为起点的所有tracklets
vector<tracklet> tracklets;

int main()
{
	form_tracklets();
	association();

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

void form_tracklets()
{
	string s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	string num;
	string s2 = ".jpg";
	trace_gray = read_trans("D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_0000.jpg");

	segmentation SEG;
	trace_draw = SEG.segment("trace", trace_gray, trace_dst, trace_minellipse);       //存储开始的信息
	pre_minellipse = trace_minellipse;      //pre_minellipse 为每次匹配中的 前帧

	minellipse[0].obj_minellipse = trace_minellipse;
	minellipse[0].pre_link.resize(trace_minellipse.size());
	for (int n = 0; n < trace_minellipse.size(); n++)
	{
		minellipse[0].pre_link[n] = 1;
	}

	int pre_num, aft_num;
	HungarianAlgorithm HungAlgo;

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
		string aft_name = "aft" + num;
		SEG.segment(aft_name, aft_gray, aft_dst, aft_minellipse);    // aft_minellipse 为每次匹配中的 后帧

		minellipse[i].obj_minellipse = aft_minellipse;
		minellipse[i].pre_link.resize(aft_minellipse.size());
		for (int n = 0; n < aft_minellipse.size(); n++)
		{
			minellipse[i].pre_link[n] = 1;
		}
		
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
				connect[a][b] = W_Euclidean * sqrt(pow(pre_minellipse[a].center.y - aft_minellipse[b].center.y, 2) + pow(pre_minellipse[a].center.x - aft_minellipse[b].center.x, 2))
					//+	 PI * abs((pre_minellipse[a].size.height / 2) *(pre_minellipse[a].size.width / 2) - (aft_minellipse[b].size.height / 2)*(aft_minellipse[b].size.height / 2) )
					;
			}
		}

		//Hungarian 匹配
		HungAlgo.Solve(connect, matching[i - 1]);

		//*********************************************************************
		//设置阈值，将距离权值较大的两个连接点强制断开,得到reliable的tracklets
		//*********************************************************************
		//
		for (int k = 0; k < connect.size(); k++)
		{
			int j = matching[i - 1][k];
			if (j != -1 && connect[k][j] > REMOTE)         //注意: Assignment[i] 的值可能取到 -1 ，使得下标越界
			{
				matching[i - 1][k] = -1;              //切断得到reliable的tracklets
			}

		}

		//标记轨迹段开始位置，（根据 i-1 帧和 i 帧之间的匹配关系）
		for (int n = 0; n < matching[i - 1].size(); n++)
		{
			if (matching[i - 1][n] != -1)
			{
				minellipse[i].pre_link[matching[i - 1][n]] = 0;
			}
		}

		Scalar color = Scalar(0, 255, 0);   //轨迹颜色
		for (int k = 0; k < matching[i - 1].size(); k++)             //根据 matching[] 结果画线
		{
			//cout << matching[i-1][k] << endl;
			if (matching[i - 1][k] != -1)
			{
				line(trace_draw, pre_minellipse[k].center, aft_minellipse[matching[i - 1][k]].center, color, 1, 8);
			}
		}

		pre_minellipse = aft_minellipse;      //后帧信息赋值给前帧
		s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	}

	namedWindow("trace", WINDOW_AUTOSIZE);
	imshow("trace", trace_draw);
	return;
}

void association()
{
	for (int i = 1; i < IM_NUM; i++)
	{
		//此时，根据matching[i-1]计算当前两帧之间的tracklets
		for (int m = 0; m < matching[i - 1].size(); m++)
		{
			if ((minellipse[i - 1].pre_link[m] == 1) && (matching[i - 1][m] != -1))    //找tracklets的 开始点 (条件是之前未匹配过，且会匹配下一个)
			{
				int sum = 0;    //计数器，用来计算轨迹连接次数
				tracklet tem;
				tem.t_s = i - 1;
				tem.s_ID = m;
				
				int cur = matching[i - 1][m];     //循环
				tem.t_e = i - 1;
				int k = m;
				//int x = i - 1;
				while (cur != -1 )
				{
					sum++;
					tem.t_e += 1;
					tem.e_ID = cur;

					tem.len += sqrt(pow(minellipse[tem.t_e - 1].obj_minellipse[k].center.y - minellipse[tem.t_e].obj_minellipse[cur].center.y, 2) + pow(minellipse[tem.t_e - 1].obj_minellipse[k].center.x - minellipse[tem.t_e].obj_minellipse[cur].center.x, 2));
					k = matching[tem.t_e - 1][k];
					if (tem.t_e > IM_NUM - 2)
					{
						//cout << "matching越界" << endl;
						break;
					}
					cur = matching[tem.t_e][cur];
				}
				tem.v = tem.len / sum;
				tracklets_formulate[i - 1].push_back(tem);
				tracklets.push_back(tem);  //添加一条轨迹
			}

		}
	}

	//******************************
	//tracklets之间进行hungarian
	//******************************
	vector<double> temp;  //临时变量，初始化使用
	temp.resize(tracklets.size());
	vector<vector<double>> track_connect;   //权值矩阵
	track_connect.resize(tracklets.size(), temp);

	const int INF = 1000;  //无穷大
	const int LENGTH = 50;

	for (int i = 0; i < tracklets.size(); i++)
	{
		for (int j = 0; j < tracklets.size(); j++)
		{
			RotatedRect pre = minellipse[tracklets[i].t_e].obj_minellipse[tracklets[i].e_ID];
			RotatedRect aft = minellipse[tracklets[j].t_s].obj_minellipse[tracklets[j].s_ID];

			if ((tracklets[i].t_e < tracklets[j].t_s) && sqrt(pow(pre.center.y - aft.center.y, 2) + pow(pre.center.x - aft.center.x, 2))<LENGTH)
			//if ((tracklets[i].t_e < tracklets[j].t_s)  )             //条件
			{
				track_connect[i][j] = abs(tracklets[i].v - tracklets[j].v);
			}
			else
			{
				track_connect[i][j] = INF;     //为不可能的tracklets匹配，设置一个较大的权值
			}
		}
	}
	
	//Hungarian 匹配
	HungarianAlgorithm HungAlgo;
	vector<int> track_matching(tracklets.size()-1);    //track_matching
	HungAlgo.Solve(track_connect, track_matching);


	for (int i = 0; i < track_matching.size(); i++)   //筛选
	{
		if (track_connect[i][track_matching[i]] == INF)
		{
			track_matching[i] = -1;                                  //断开
		}
	}
	
	Scalar color = Scalar(0, 0, 255);   //轨迹颜色
	for (int k = 0; k < track_matching.size(); k++)             //根据 track_matching[] 结果画线
	{
		if (track_matching[k] != -1)
		{
			line(trace_draw, minellipse[tracklets[k].t_e].obj_minellipse[tracklets[k].e_ID].center, minellipse[tracklets[track_matching[k]].t_s].obj_minellipse[tracklets[track_matching[k]].s_ID].center, color, 1, 8);
		}
	}

	namedWindow("trace", WINDOW_AUTOSIZE);
	imshow("trace", trace_draw);

	return;
	
}
