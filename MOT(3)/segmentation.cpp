#include "segmentation.h"

#define thr 46  //初始化阈值  
#define IM_SIZE 3   //分割时，筛选尺寸
//#define IM_ROUNDNESS 0.4   //分割时，筛选圆度

segmentation::segmentation(){}
segmentation::~segmentation(){}

Mat segmentation::segment(string im_name, Mat &im_gray, Mat &im_dst, vector<RotatedRect> &im_minEllipse )
{
	//hierarchy是一个向量，向量内每个元素都是一个包含4个int型的数组。向量hierarchy内的元素和轮廓向量contours内的元素是一一对应的，向量的容量相同。
	//hierarchy内每个元素的4个int型变量是hierarchy[i][0] ~hierarchy[i][3]，分别表
	//示当前轮廓 i 的后一个轮廓、前一个轮廓、父轮廓和内嵌轮廓的编号索引
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	//vector<double> im_roundness;

	//cout << "##################  threshold:   " << thr << "    #########################" << endl;
	threshold(im_gray, im_dst, thr, 255, THRESH_BINARY);  //阈值函数，图像分割  THRESH_TOZERO\THRESH_BINARY\.....
	findContours(im_dst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));   //找轮廓,Point是偏移量

	//对轮廓进行一些预处理操作
	vector<vector<Point>>::iterator contours_it;     //轮廓指针
	for (contours_it = contours.begin(); contours_it != contours.end();)    ////删去小于10个像素的轮廓
	{
		if ((*contours_it).size() < 6)      //删去小于10个像素的边缘
		{
			contours_it = contours.erase(contours_it);    //若删除成功，则指针已经+1
		}
		else
		{
			++contours_it;
		}
	} 
	 
	//im_roundness.resize(contours.size());    //  每个轮廓的圆形度
	im_minEllipse.resize(contours.size());    //RotatedRect是一个存储旋转矩形的类，用于存储椭圆拟合函数返回的结果

	for (int i = 0; i < contours.size(); i++)   //对每一个轮廓画最小包围圆
	{
		//对指定的点集进行多边形逼近的函数,连续光滑曲线折线化，对图像轮廓点进行多边形拟合。
		//approxPolyDP(contours[i], contours[i], 1, true);    //true代表曲线是闭合的
		if (contours[i].size() > 5)          //至少6个点画椭圆
		{
			im_minEllipse[i] = fitEllipse(contours[i]);    //最小外接椭圆
		//	im_roundness[i] = (4 * CV_PI * contourArea(contours[i])) / (arcLength(contours[i], true)  *arcLength(contours[i], true));//圆度
		}
		else
			cout << "像素点数量<5" << endl;
	}

	//..按条件筛选。。。。。。。。。。。。。。。。。。。。。。
	//vector<double>::iterator roundness_it = im_roundness.begin();    //  圆形度指针
	vector<RotatedRect>::iterator minEllipse_it = im_minEllipse.begin();    //椭圆指针
	for (contours_it = contours.begin(); contours_it != contours.end();)    //it++要写到for里面，否则不能删除连续值
	{
		//if ((*minEllipse_it).size.width < IM_SIZE || (*roundness_it) < IM_ROUNDNESS)
		if ((*minEllipse_it).size.width < IM_SIZE && (*minEllipse_it).size.height < IM_SIZE)
		{
			contours_it = contours.erase(contours_it);
			//roundness_it = im_roundness.erase(roundness_it);
			minEllipse_it = im_minEllipse.erase(minEllipse_it);
		}
		else
		{
			++contours_it;
		//	++roundness_it;
			++minEllipse_it;
		}
	}

	//画图
	Mat drawing = Mat::zeros(im_dst.size(), CV_8UC3);   //重绘一个三通道的，背景为黑色的图
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color_R = Scalar(0, 255, 255);
		Scalar color = Scalar(255, 0, 0);
		drawContours(drawing, contours, i, color_R, 1, 8, vector<Vec4i>(), 0, Point());
		//ellipse(drawing, im_minEllipse[i], color, 1, 8);   //外接椭圆，记录每个细胞信息
	//	cout << (i + 1) << im_minEllipse[i].center << "   minor axis:  " << setprecision(6) << im_minEllipse[i].size.width << "  long axis:  " << setprecision(6) << im_minEllipse[i].size.height << "  roundness:" << setprecision(6) << im_roundness[i] << endl;
	}
	//cout << endl << endl;
	/// 在窗体中显示结果
	//namedWindow(im_name, WINDOW_AUTOSIZE);
	//imshow(im_name, drawing);
	return drawing;
}
