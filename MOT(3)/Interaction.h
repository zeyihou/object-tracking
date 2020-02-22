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
	bool judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse);
};

#endif