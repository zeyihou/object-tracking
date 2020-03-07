#ifndef INTERACTION_H   //人机协作
#define INTERACTION_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>

using namespace std;
using namespace cv;

class Interaction
{
public:
	Interaction();
	~Interaction();
	vector<RotatedRect> Solve(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name); //判断、处理重叠
private:
	Mat SRC, DST, DST_gray, DST_dst;  //鼠标划线切割的原图、目标图
	Mat I_img, I_img_gray, I_img_dst, show;   //用于存储图片

	//*************Images的size在构造函数中初始化********************//
	vector<Mat> Images; //  //模板类vector，用于放置类型为Mat的元素  

	void line_mouse(int events, int x, int y, int flags);   //鼠标划线响应函数
	static void linemouse(int event, int x, int y, int flags, void* ustc); //鼠标划线响应函数
	Mat combineImages(vector<Mat> imgs, int col, int row, bool hasMargin);
	bool judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name);
};

#endif