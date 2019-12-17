#ifndef SEGMENTATION_H
#define SEGMENTATION_H

#include<iostream>
#include<iomanip>
#include<vector>
#include<string>
#include<opencv2/opencv.hpp>
#include<cmath>
#include<stdlib.h>

using namespace std;
using namespace cv;

class segmentation
{
public:
	segmentation();
	~segmentation();
	Mat segment(string im_name, Mat &im_gray, Mat &im_dst, vector<RotatedRect> &im_minEllipse);  //·Ö¸îº¯Êý
};


#endif