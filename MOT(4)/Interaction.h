#ifndef INTERACTION_H   //�˻�Э��
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
	vector<RotatedRect> Solve(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name); //�жϡ������ص�
private:
	Mat SRC, DST, DST_gray, DST_dst;  //��껮���и��ԭͼ��Ŀ��ͼ
	Mat I_img, I_img_gray, I_img_dst, show;   //���ڴ洢ͼƬ

	//*************Images��size�ڹ��캯���г�ʼ��********************//
	vector<Mat> Images; //  //ģ����vector�����ڷ�������ΪMat��Ԫ��  

	void line_mouse(int events, int x, int y, int flags);   //��껮����Ӧ����
	static void linemouse(int event, int x, int y, int flags, void* ustc); //��껮����Ӧ����
	Mat combineImages(vector<Mat> imgs, int col, int row, bool hasMargin);
	bool judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name);
};

#endif