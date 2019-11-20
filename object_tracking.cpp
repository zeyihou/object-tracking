#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<opencv2/opencv.hpp>
#include<math.h>

using namespace std;
using namespace cv;

//typedef vector<vector<Point>> CON;  //轮廓类型
//typedef vector<RotatedRect>  MINELLIPSE;   ////RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果

//函数声明
void my_segmentation(string im_name, Mat im_gray, Mat im_dst, vector<vector<Point>> contours, vector<RotatedRect> im_minEllipse, vector<double> im_roundness);  //分割函数

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

int thr = 40;  //初始化阈值
#define IM_SIZE 5   //筛选尺寸
#define IM_ROUNDNESS 0.4   //筛选圆度

int main()
{
	pre = imread("D:\\test.jpg");
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
	aft = imread("D:\\test_0.jpg");
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
	
	my_segmentation("pre",pre_gray,pre_dst,pre_contours,pre_minellipse,pre_roundness);  //分割
	my_segmentation("aft",aft_gray,aft_dst,aft_contours,aft_minellipse,aft_roundness);

	
	waitKey(0);
	return 0;
}

void my_segmentation(string im_name,Mat im_gray, Mat im_dst, vector<vector<Point>> contours, vector<RotatedRect> im_minEllipse, vector<double> im_roundness)
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
		}
		else
			cout << "像素点数量<5"<<endl;
		im_roundness[i] = (4 * CV_PI * contourArea(contours[i])) / ( arcLength(contours[i], true)  *arcLength(contours[i], true)  );//圆度
	}

	//..按条件筛选。。。。。。。。。。。。。。。。。。。。。。
	vector<double>::iterator roundness_it = im_roundness.begin();    //  圆形度指针
	vector<RotatedRect>::iterator minEllipse_it = im_minEllipse.begin();    //椭圆指针
	for (contours_it = contours.begin(); contours_it != contours.end();)    //it++要写到for里面，否则不能删除连续值
	{
		if ((*minEllipse_it).size.width < IM_SIZE || *roundness_it < IM_ROUNDNESS)
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
		cout << (i + 1) <<im_minEllipse[i].center <<"   minor axis:  " << setprecision(6) << im_minEllipse[i].size.width << "  long axis:  " << setprecision(6) << im_minEllipse[i].size.height << "  roundness:" << setprecision(6) << im_roundness[i] << endl;
	}
	cout << endl << endl;

	/// 在窗体中显示结果
	namedWindow(im_name, WINDOW_AUTOSIZE);
	imshow(im_name, drawing);
}
