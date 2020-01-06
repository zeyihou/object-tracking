#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>

#include "segmentation.h"
#include "Hungarian.h"

//自定义结构体
struct obj
{
	vector<RotatedRect> obj_minellipse;      //obj_minellipse   存储第i幅图片的信息
	vector<int> pre_link;    //用来标记每一个目标是否可作为轨迹段起点
};
struct tracklet      //轨迹段
{
	int t_s;   //开始帧   matching的下标
	int s_ID;   //tracklets起点在开始帧中的ID
	int t_e;   //结束帧
	int e_ID;   //tracklets终点在结束帧中的ID
	double len = 0;  //轨迹长度
	double v = 0;   //运动速度
};

using namespace std;
using namespace cv;

class tracker
{
public:
	tracker();
	~tracker();
	void form_tracklets();          //第一轮逐帧匹配,形成reliable tracklets
	void association();           //数据关联（对tracklets匈牙利算法匹配）

private:
	Mat read_trans(string path);    //读取图片+转换成灰度图
	//变量
	//Mat pre,pre_gray,pre_dst;
	Mat aft, aft_gray, aft_dst;
	Mat trace_gray, trace_dst, trace_draw;
	vector<RotatedRect> pre_minellipse;   //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果
	vector<RotatedRect> aft_minellipse;
	vector<RotatedRect> trace_minellipse;     //存储第一幅图像的信息

public:
	//vector<obj> minellipse(IM_NUM);    //存储所有帧信息
	vector<obj> minellipse;
	
	//vector<int> matching;          //记录匹配结果，如果没有则为-1
	//vector<vector<int>> matching(IM_NUM - 1);    //matching[i]  记录第i幅图片和它后面一张的匹配结果
	vector<vector<int>> matching;

	vector<int> track_matching;       //tracklets之间的匹配

	//vector<vector<tracklet>> tracklets_formulate(IM_NUM);   //存储第一轮逐帧匹配后的tracklets，tracklets[0]代表第一帧为起点的所有tracklets
	vector<vector<tracklet>> tracklets_formulate;
	vector<tracklet> tracklets;
};



#endif