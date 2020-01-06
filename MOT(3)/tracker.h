#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>

#include "segmentation.h"
#include "Hungarian.h"

//�Զ���ṹ��
struct obj
{
	vector<RotatedRect> obj_minellipse;      //obj_minellipse   �洢��i��ͼƬ����Ϣ
	vector<int> pre_link;    //�������ÿһ��Ŀ���Ƿ����Ϊ�켣�����
};
struct tracklet      //�켣��
{
	int t_s;   //��ʼ֡   matching���±�
	int s_ID;   //tracklets����ڿ�ʼ֡�е�ID
	int t_e;   //����֡
	int e_ID;   //tracklets�յ��ڽ���֡�е�ID
	double len = 0;  //�켣����
	double v = 0;   //�˶��ٶ�
};

using namespace std;
using namespace cv;

class tracker
{
public:
	tracker();
	~tracker();
	void form_tracklets();          //��һ����֡ƥ��,�γ�reliable tracklets
	void association();           //���ݹ�������tracklets�������㷨ƥ�䣩

private:
	Mat read_trans(string path);    //��ȡͼƬ+ת���ɻҶ�ͼ
	//����
	//Mat pre,pre_gray,pre_dst;
	Mat aft, aft_gray, aft_dst;
	Mat trace_gray, trace_dst, trace_draw;
	vector<RotatedRect> pre_minellipse;   //RotatedRect��һ���洢��ת���ε��࣬���ڴ洢��Բ��Ϻ������صĽ��
	vector<RotatedRect> aft_minellipse;
	vector<RotatedRect> trace_minellipse;     //�洢��һ��ͼ�����Ϣ

public:
	//vector<obj> minellipse(IM_NUM);    //�洢����֡��Ϣ
	vector<obj> minellipse;
	
	//vector<int> matching;          //��¼ƥ���������û����Ϊ-1
	//vector<vector<int>> matching(IM_NUM - 1);    //matching[i]  ��¼��i��ͼƬ��������һ�ŵ�ƥ����
	vector<vector<int>> matching;

	vector<int> track_matching;       //tracklets֮���ƥ��

	//vector<vector<tracklet>> tracklets_formulate(IM_NUM);   //�洢��һ����֡ƥ����tracklets��tracklets[0]�����һ֡Ϊ��������tracklets
	vector<vector<tracklet>> tracklets_formulate;
	vector<tracklet> tracklets;
};



#endif