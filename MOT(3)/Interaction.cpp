//**********************************************************//
//                  人机协作         
//         Human-Machine Collaboration
//        判断前后两帧目标的重叠和碰撞 
//*********************************************************//

#include "Interaction.h"
#include "segmentation.h"

#include <opencv2/opencv.hpp>             //鼠标响应
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

Interaction::Interaction(){}
Interaction::~Interaction(){}

void line_mouse(int event, int x, int y, int flags, void* ustc);
Mat combineImages(vector<Mat> imgs, int col, int row, bool hasMargin);

Mat SRC, DST, DST_gray, DST_dst;  //鼠标划线切割的原图、目标图
Mat I_img, I_img_gray, I_img_dst, show;   //用于存储图片
vector<Mat> Images(2);//模板类vector，用于放置2个类型为Mat的元素，即2张图片 

void line_mouse(int event, int x, int y, int flags, void* ustc)     //鼠标划线响应函数
{
	static CvPoint pre_pt;
	static CvPoint cur_pt;
	segmentation SegAlgo;
	vector<RotatedRect> temp_minellipse;

	if (event == EVENT_LBUTTONDOWN)
	{
		pre_pt = Point(x, y);
	}
	else if (event == EVENT_MOUSEMOVE && (flags&EVENT_FLAG_LBUTTON))
	{
		Point pt(x, y);
		line(SRC, pre_pt, pt, Scalar(0, 0, 0), 1, 4, 0);   //采用背景颜色黑色进行分割划线
		pre_pt = pt;
		Images[0] = SRC;
		cvtColor(SRC, I_img_gray, COLOR_BGR2GRAY);   //转换为灰度图
		show = SegAlgo.segment("show", I_img_gray, I_img_dst, temp_minellipse);    //阈值分割
		Images[1] = show;
		Mat combine = combineImages(Images, 2, 1, false);  //组合多张图片
		imshow("人工分割", combine);//调用一个窗口显示多图函数
		//imshow("人工分割", SRC);
		SRC.copyTo(DST);
	}
}

Mat combineImages(vector<Mat> imgs,//@parameter1:需要显示的图像组 
	int col,//parameter2:显示的列数
	int row, //parameter3:显示的行数
	bool hasMargin)
{
	//parameter4:是否设置边框
	int imgAmount = imgs.size();//获取需要显示的图像数量
	int width = imgs[0].cols;//本函数默认需要显示的图像大小相同
	int height = imgs[0].rows;//获取图像宽高
	int newWidth, newHeight;//新图像宽高
	if (!hasMargin){
		newWidth = col*imgs[0].cols;//无边框，新图像宽/高=原图像宽/高*列/行数
		newHeight = row*imgs[0].rows;
	}
	else{
		newWidth = (col + 1) * 20 + col*width;//有边框，要将上边框的尺寸，这里设置边框为20px
		newHeight = (row + 1) * 20 + row*height;
	}

	Mat newImage(newHeight, newWidth, CV_8UC3, Scalar(255, 255, 255));//显示创建设定尺寸的新的大图像；色深八位三通道；填充为白色
	//imshow("new", newImage);

	int x, y, imgCount;//x列号，y行号，imgCount图片序号
	if (hasMargin){//有边框
		imgCount = 0;
		x = 0; y = 0;
		while (imgCount < imgAmount){
			Mat imageROI = newImage(Rect(x*width + (x + 1) * 20, y*height + (y + 1) * 20, width, height));//创建感兴趣区域
			imgs[imgCount].copyTo(imageROI);//将图像复制到大图中
			imgCount++;
			if (x == (col - 1)){
				x = 0;
				y++;
			}
			else{
				x++;
			}//移动行列号到下一个位置
		}
	}
	else{//无边框
		imgCount = 0;
		x = 0; y = 0;
		while (imgCount < imgAmount){
			Mat imageROI = newImage(Rect(x*width, y*height, width, height));
			imgs[imgCount].copyTo(imageROI);
			imgCount++;
			if (x == (col - 1)){
				x = 0;
				y++;
			}
			else{
				x++;
			}
		}
	}
	return newImage;//返回新的组合图像
};

bool Interaction::judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse)
{
	if (pre_minellipse.size() != aft_minellipse.size())
		return true;
	else
		return false;
}

vector<RotatedRect> Interaction::Solve(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name)
{
	Mat show;
	vector<RotatedRect> temp_minellipse;
	segmentation SegAlgo;

	if ( judge(pre_minellipse, aft_minellipse) )   //检测是否发生重叠、碰撞的条件
	{
		I_img = imread(aft_name);   //读取原图
		if (I_img.empty())
		{
			//检查是否读取图像
			cout << "error! 图片读取失败\n";
			waitKey(0);
		}
		else
		{
			namedWindow("人工分割", WINDOW_NORMAL);  //由于需要修改窗口大小，所以选择WINDOW_NORMAL而不是WINDOW_AUTOSIZE
			SRC = I_img;
			Images[0] = SRC;
			//imshow("人工分割", SRC);

			cvtColor(I_img, I_img_gray, COLOR_BGR2GRAY);   //转换为灰度图
			show = SegAlgo.segment(aft_name, I_img_gray, I_img_dst, temp_minellipse);    //阈值分割
			Images[1] = show;
			Mat combine = combineImages(Images, 2, 1, false);  //组合多张图片
			imshow("人工分割", combine);//调用一个窗口显示多图函数
			//imshow("灰度", show);

			SRC.copyTo(DST);
			setMouseCallback("人工分割", line_mouse, 0);
			waitKey(0);
		}
		
		cout << "碰撞/重叠：" << aft_name << endl;    //打印碰撞重叠帧
		//imshow("last", DST);  //显示上一次划分后的原图
		vector<RotatedRect> dst_minellipse;
		cvtColor(DST, DST_gray, COLOR_BGR2GRAY);   //转换为灰度图
		SegAlgo.segment(aft_name, DST_gray, DST_dst, dst_minellipse);    //阈值分割
		//cout << "~Interaction aft size:" << aft_minellipse.size() << endl;
		//cout << "Interaction dst size:" << dst_minellipse.size() << endl;
		return dst_minellipse;
	}
	
	return aft_minellipse;
}