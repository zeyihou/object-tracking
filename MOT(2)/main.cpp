#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>
#include <Python.h>

#include "segmentation.h"
#include "Hungarian.h"
#include "tracker.h"
#include "MatPlot.h"

#include <opencv2/opencv.hpp>             //鼠标响应
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

using namespace std;
using namespace cv;

Mat org, dst, img, tmp;
void on_mouse(int event, int x, int y, int flags, void *ustc) 
{
	static Point pre_pt = (-1, -1);//初始坐标  
	static Point cur_pt = (-1, -1);//实时坐标  
	char temp[16];
	if (event == CV_EVENT_LBUTTONDOWN) {//左键按下，读取初始坐标，并在图像上该点处划圆  

		org.copyTo(img);
		sprintf_s(temp, "(%d,%d)", x, y);   //将数据格式化输出到字符串
		pre_pt = Point(x, y);
		putText(img, temp, pre_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 0), 1, 8);//在窗口上显示坐标  
		circle(img, pre_pt, 2, Scalar(255, 0, 0), CV_FILLED, CV_AA, 0);//划圆  
		imshow("img", img);
	}
	else if (event == CV_EVENT_MOUSEMOVE && !(flags & CV_EVENT_FLAG_LBUTTON)){//左键没有按下的情况下鼠标移动的处理函数  
		img.copyTo(tmp);
		sprintf_s(temp, "(%d,%d)", x, y);   //将数据格式化输出到字符串
		cur_pt = Point(x, y);
		putText(tmp, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 0));//只是实时显示鼠标移动的坐标  
		imshow("img", tmp);
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON)){ //左键按下时，鼠标移动，则在图像上划矩形  
		img.copyTo(tmp);
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(tmp, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 0));
		rectangle(tmp, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//在临时图像上实时显示鼠标拖动时形成的矩形  
		imshow("img", tmp);
	}
	else if (event == CV_EVENT_LBUTTONUP) {//左键松开，将在图像上划矩形  
		org.copyTo(img);
		sprintf_s(temp, "(%d,%d)", x, y);
		cur_pt = Point(x, y);
		putText(img, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(255, 255, 0));
		circle(img, pre_pt, 2, Scalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		rectangle(img, pre_pt, cur_pt, Scalar(0, 255, 0, 0), 1, 8, 0);//根据初始点和结束点，将矩形画到img上  
		imshow("img", img);
		img.copyTo(tmp);
		//截取矩形包围的图像，并保存到dst中  
		int width = abs(pre_pt.x - cur_pt.x);
		int height = abs(pre_pt.y - cur_pt.y);
		if (width == 0 || height == 0) {
			printf("width == 0 || height == 0");
			return;
		}
		dst = org(Rect(min(cur_pt.x, pre_pt.x), min(cur_pt.y, pre_pt.y), width, height));
		namedWindow("dst");          //即新建立一个窗口进行输出结果
		imshow("dst", dst);
		waitKey(0);
	}
}

Mat SRC, DST;

void line_mouse(int event, int x, int y, int flags, void* ustc)
{
	static CvPoint pre_pt = { -1, -1 };
	static CvPoint cur_pt = { -1, -1 };

	if (event == CV_EVENT_LBUTTONDOWN)
	{
		DST.copyTo(SRC);
		pre_pt = cvPoint(x, y);
		cv::circle(SRC, pre_pt, 3, cvScalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		imshow("src", SRC);
		SRC.copyTo(DST);
	}
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		DST.copyTo(SRC);
		cur_pt = cvPoint(x, y);
		cv::line(SRC, pre_pt, cur_pt, cvScalar(0, 255, 0, 0), 1, CV_AA, 0);
		imshow("src", SRC);
	}
	else if (event == CV_EVENT_LBUTTONUP)
	{
		DST.copyTo(SRC);
		cur_pt = cvPoint(x, y);
		cv::circle(SRC, cur_pt, 3, cvScalar(255, 0, 0, 0), CV_FILLED, CV_AA, 0);
		cv::line(SRC, pre_pt, cur_pt, cvScalar(0, 255, 0, 0), 1, CV_AA, 0);
		imshow("src", SRC);
		SRC.copyTo(DST);
	}
}

int main()
{
	tracker trackerAlgo;
	trackerAlgo.form_tracklets();   //形成轨迹段tracklets
	trackerAlgo.form_trajectory();      //tracklets之间相连接
	MatPlot PlotAlgo;
	PlotAlgo.Plot_trajectory_3D(trackerAlgo.motion_location);   //绘制3D轨迹图
	waitKey(0);

	//鼠标事件响应
	org = imread("D:\\pre.jpg");
	org.copyTo(img);
	org.copyTo(tmp);
	namedWindow("img");//定义一个img窗口  
	setMouseCallback("img", on_mouse, 0);//调用回调函数  
	imshow("img", img);
	waitKey(0);


	//namedWindow("src", CV_WINDOW_AUTOSIZE);
	//SRC = imread("D:\\pre.jpg", 1);
	//SRC.copyTo(DST);
	//setMouseCallback("src", line_mouse, 0);
	//imshow("src", SRC);
	//waitKey(0);

	return 0;
}

