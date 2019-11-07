#include<iostream>
#include<opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int Otsu_Threshold(const Mat image);   //用来计算otsu的自适应阈值
Mat Otsu(const Mat image);//otsu图像分割算法,使用ostu阈值进行二值分割

Mat src, src_gray, dst;  //全局变量
int thr;  //初始化阈值

int main()
{
	src = imread("D:\\test.jpg");
	if (src.empty())
	{
		//检查是否读取图像
		cout << "error! 图片读取失败\n";
		waitKey(0);
		getchar();
		return -1;
	}
	
	namedWindow("原图", WINDOW_AUTOSIZE);  // 创建窗体
	imshow("原图", src);
  dst=Otsu(src);
  namedWindow("分割", WINDOW_AUTOSIZE);
  imshow("分割",dst);
	waitKey(0);

	return 0;
}

/*
像素点数占整幅图像的比例记为ω，其平均灰度记为μ
ω0=N0/ M×N    (1)
ω1=N1/ M×N    (2)
N0+N1=M×N    (3)
ω0+ω1=1　　　 (4)
μ=ω0*μ0+ω1*μ1 (5)
g=ω0(μ0-μ)^2+ω1(μ1-μ)^2 (6)
将式(5)代入式(6),得到等价公式:
g=ω0ω1(μ0-μ1)^2   (7)　这个就是类间方差的公式表述
*/
int Otsu_Threshold(const Mat image)
{
	int thr=0;  //ostu算法阈值
	int Histogram[256] = {0}; //灰度直方图，下标是灰度值，保存内容是灰度值对应的像素点总数
	double w0, w1, u0, u1;   //前景、后景：像素点比例、灰度平均值
	double var, Max_variance = 0;   //方差，最大方差
	Mat img;  //灰度图

	cvtColor(image, img, COLOR_BGR2GRAY);  //转换为灰度图
	uchar *p_data;   //unsigned char，8 bit, 表示0-255
	int total_pixel = img.rows*img.cols;   //像素点总数

	//计算灰度直方图分布，Histogram数组下标是灰度值，保存内容是灰度值对应像素点数
	for (int i = 0; i < img.rows; i++)
	{
		p_data = img.ptr<uchar>(i);  //获取第i行首地址
		for (int j = 0; j < img.cols; j++)
			Histogram[ p_data[j] ]++;
	}

	//遍历0-255每一阶灰度，计算前景，背景，求得最大方差
	for (int k = 0; k < 256; k++)
	{
		w0 = 0;  //每次循环先初始化
		w1 = 0;
		u0 = 0;
		u1 = 1;

		//*********前景************
		for (int i = 0; i <= k; i++)
		{
			w0 += Histogram[i];   //前景 像素点总数
			u0 += (i*Histogram[i]);   //前景 像素点总灰度和
		}
		u0 = u0 / w0;   //前景 平均灰度
		w0 = w0 / total_pixel;   //前景 像素点所占比例

		//*********背景************
		for (int j = k + 1; j < 256; j++)
		{
			w1 += Histogram[j];   //背景 像素点总数
			u1 += (j*Histogram[j]);   //背景 像素点总灰度和
		}
		u1 = u1 / w1;   //背景景 平均灰度
		w1 = w1 / total_pixel;   //前景 像素点所占比例

		var = w0*w1*(u0 - u1)*(u0 - u1);   //本轮循环方差
		if (Max_variance < var)
		{
			Max_variance = var;
			thr = k;     //当前最大方差下的灰度阈值
		}

	}

	return thr;
}

Mat Otsu(const Mat image)
{
	int thr = Otsu_Threshold(image);   //Otsu阈值计算
	cout << "最大方差为：" << thr << endl;

	Mat img_output = image.clone();
	cvtColor(img_output, img_output, COLOR_BGR2GRAY);  //转换为灰度图
	//threshold(img_output, img_output, 50, 255, THRESH_BINARY);  //阈值函数，图像分割
	threshold(img_output, img_output, thr, 255, THRESH_TOZERO);  //阈值函数，图像分割

	return img_output;

}
