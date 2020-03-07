//**********************************************************//
//                  人机协作         
//         Human-Machine Collaboration
//        判断前后两帧目标的重叠和碰撞 
//*********************************************************//

#include "Interaction.h"
#include "segmentation.h"
#include "Hungarian.h"

#include <opencv2/opencv.hpp>             //鼠标响应
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

#define THR_AREA 1.6    //面积突变阈值
#define REMOTE 20   //相邻帧之前匹配的距离上限，距离超过REMOTE，则进行轨迹切断

Interaction::Interaction()
{
	Images.resize(2);    //确定要拼接显示的图片张数
}
Interaction::~Interaction(){}

void Interaction::linemouse(int event, int x, int y, int flags, void* ustc)     //鼠标划线响应函数
{
	Interaction* temp = reinterpret_cast<Interaction*>(ustc);
	temp->line_mouse(event, x, y, flags);
}

void Interaction::line_mouse(int event, int x, int y, int flags)    //鼠标划线响应函数
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

Mat Interaction::combineImages(vector<Mat> imgs,//@parameter1:需要显示的图像组 
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

//判断发生碰撞\重叠
bool Interaction::judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name)
{
	//cout << "pre" << pre_minellipse.size() << endl;
	//cout << "aft" << aft_minellipse.size() << endl;
	Mat img = imread(aft_name);   //读取原图,为获得图片尺寸
	int rows = img.rows;
	int cols = img.cols;
	struct part          //用于存储每个小区域信息的结构体
	{
		int num;     //细胞数目
		vector<RotatedRect> minellipse;  //细胞信息
	};
	vector<part> pre_part(9);
	vector<part> aft_part(9);
	vector<RotatedRect>::iterator it;

	vector<part>::iterator part_it;  //初始化
	for (part_it = pre_part.begin(); part_it != pre_part.end(); part_it++)
	{
		part_it->num = 0;
	}
	for (part_it = aft_part.begin(); part_it != aft_part.end(); part_it++)
	{
		part_it->num = 0;
	}
	//cout << "pre  size:" << pre_minellipse.size() << endl;

	//区域划分排列情况
	// [0 3 6]
	// [1 4 7]
	// [2 5 8]
	//前帧pre情况统计
	for (it = pre_minellipse.begin(); it != pre_minellipse.end(); it++)
	{
		if ( it->center.x <= (cols / 3) )              //第一列
		{
			if ( it->center.y <= (rows / 3) )        //(1,1)
			{
				pre_part[0].num++;
				pre_part[0].minellipse.push_back(*it);
			}
			else if ( it->center.y <= (rows / 3 * 2) )  //(2,1)
			{
				pre_part[1].num++;
				pre_part[1].minellipse.push_back(*it);
			}
			else                                          //(3,1)      
			{
				pre_part[2].num++;
				pre_part[2].minellipse.push_back(*it);
			}
		}
		else if ( it->center.x <= (cols / 3 * 2) )          //第二列
		{
			if (it->center.y <= (rows / 3))        //(1,2)
			{
				pre_part[3].num++;
				pre_part[3].minellipse.push_back(*it);
			}
			else if (it->center.y <= (rows / 3 * 2))  //(2,2)
			{
				pre_part[4].num++;
				pre_part[4].minellipse.push_back(*it);
			}
			else                                          //(3,2)      
			{
				pre_part[5].num++;
				pre_part[5].minellipse.push_back(*it);
			}
		}
		else                                           //第三列
		{
			if (it->center.y <= (rows / 3))        //(1,3)
			{
				pre_part[6].num++;
				pre_part[6].minellipse.push_back(*it);
			}
			else if (it->center.y <= (rows / 3 * 2))  //(2,3)
			{
				pre_part[7].num++;
				pre_part[7].minellipse.push_back(*it);
			}
			else                                          //(3,3)      
			{
				pre_part[8].num++;
				pre_part[8].minellipse.push_back(*it);
			}
		}
	}

	//后帧aft情况统计
	for (it = aft_minellipse.begin(); it != aft_minellipse.end(); it++)
	{
		if (it->center.x <= (cols / 3))              //第一列
		{
			if (it->center.y <= (rows / 3))        //(1,1)
			{
				aft_part[0].num += 1;
				aft_part[0].minellipse.push_back(*it);
			}
			else if (it->center.y <= (rows / 3 * 2))  //(2,1)
			{
				aft_part[1].num += 1;
				aft_part[1].minellipse.push_back(*it);
			}
			else                                          //(3,1)      
			{
				aft_part[2].num += 1;
				aft_part[2].minellipse.push_back(*it);
			}
		}
		else if (it->center.x <= (cols / 3 * 2))          //第二列
		{
			if (it->center.y <= (rows / 3))        //(1,2)
			{
				aft_part[3].num += 1;
				aft_part[3].minellipse.push_back(*it);
			}
			else if (it->center.y <= (rows / 3 * 2))  //(2,2)
			{
				aft_part[4].num += 1;
				aft_part[4].minellipse.push_back(*it);
			}
			else                                          //(3,2)      
			{
				aft_part[5].num += 1;
				aft_part[5].minellipse.push_back(*it);
			}
		}
		else                                           //第三列
		{
			if (it->center.y <= (rows / 3))        //(1,3)
			{
				aft_part[6].num += 1;
				aft_part[6].minellipse.push_back(*it);
			}
			else if (it->center.y <= (rows / 3 * 2))  //(2,3)
			{
				aft_part[7].num += 1;
				aft_part[7].minellipse.push_back(*it);
			}
			else                                          //(3,3)      
			{
				aft_part[8].num += 1;
				aft_part[8].minellipse.push_back(*it);
			}
		}
	}

	HungarianAlgorithm HungAlgo;  //局部匈牙利算法匹配变量
	
	//判断发生重叠/碰撞的条件
	//循环检查所有局部区域，如果任一区域满足条件，则返回true
	for (int k = 0; k < 9; k++)    
	{
		if ( (pre_part[k].num > aft_part[k].num) && (1) )  //局部区域数目减少（可能原因：1、碰撞 2、出边界 3、没有从背景中检测分割）
		{
			//小区域局部匈牙利匹配
			//初始化权值矩阵
			int pre_num = pre_part[k].minellipse.size();
			int aft_num = aft_part[k].minellipse.size();
			vector<double> temp;  //临时变量，初始化使用
			temp.resize(aft_num);
			vector<vector<double>> connect;    //权值矩阵
			connect.resize(pre_num, temp);
			vector<int> matching;
			for (int a = 0; a < pre_num; a++)    //初始化，要设计各种特征的权重超参数
			{
				for (int b = 0; b < aft_num; b++)
				{
					connect[a][b] = sqrt(pow(pre_part[k].minellipse[a].center.y - aft_part[k].minellipse[b].center.y, 2) + pow(pre_part[k].minellipse[a].center.x - aft_part[k].minellipse[b].center.x, 2));
				}
			}
			HungAlgo.Solve(connect, matching);   //Hungarian 匹配
			//cout << endl;
			//cout << "pre:" << pre_part[k].num << "  ";
			//cout << "matching:";
			for (int i = 0; i < matching.size(); i++)
			{
				//*********************************************************************
				//设置阈值，将距离权值较大的两个连接点强制断开,得到reliable的tracklets
				//*********************************************************************
				if (matching[i] != -1 && connect[i][matching[i]] > REMOTE)
				{
					matching[i] = -1;
				}

				//cout << matching[i] << ",";

				//判断局部区域面积发生突变
				if ((matching[i] != -1) && ((aft_part[k].minellipse[matching[i]].size.height * aft_part[k].minellipse[matching[i]].size.width) / (pre_part[k].minellipse[i].size.height * pre_part[k].minellipse[i].size.width) > THR_AREA) )
				{
					cout << "区域：" << (k + 1) << endl;  //输出满足条件的第一个局部区域
					return true;
				}
			 }
			//cout << "区域：" << (k + 1) << endl;  //输出满足条件的第一个局部区域
			//return true;
		}


	}


	return false;   //若所有局部区域均不满足条件，则返回false；
}

vector<RotatedRect> Interaction::Solve(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name)
{
	Mat show;
	vector<RotatedRect> temp_minellipse;
	segmentation SegAlgo;

	if (judge(pre_minellipse, aft_minellipse, aft_name) )   //检测是否发生重叠、碰撞的条件
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
			setMouseCallback("人工分割", Interaction::linemouse, this);
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