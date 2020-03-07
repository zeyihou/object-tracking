//**********************************************************//
//                  �˻�Э��         
//         Human-Machine Collaboration
//        �ж�ǰ����֡Ŀ����ص�����ײ 
//*********************************************************//

#include "Interaction.h"
#include "segmentation.h"
#include "Hungarian.h"

#include <opencv2/opencv.hpp>             //�����Ӧ
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

#define THR_AREA 1.6    //���ͻ����ֵ
#define REMOTE 20   //����֮֡ǰƥ��ľ������ޣ����볬��REMOTE������й켣�ж�

Interaction::Interaction()
{
	Images.resize(2);    //ȷ��Ҫƴ����ʾ��ͼƬ����
}
Interaction::~Interaction(){}

void Interaction::linemouse(int event, int x, int y, int flags, void* ustc)     //��껮����Ӧ����
{
	Interaction* temp = reinterpret_cast<Interaction*>(ustc);
	temp->line_mouse(event, x, y, flags);
}

void Interaction::line_mouse(int event, int x, int y, int flags)    //��껮����Ӧ����
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
		line(SRC, pre_pt, pt, Scalar(0, 0, 0), 1, 4, 0);   //���ñ�����ɫ��ɫ���зָ��
		pre_pt = pt;
		Images[0] = SRC;
		cvtColor(SRC, I_img_gray, COLOR_BGR2GRAY);   //ת��Ϊ�Ҷ�ͼ
		show = SegAlgo.segment("show", I_img_gray, I_img_dst, temp_minellipse);    //��ֵ�ָ�
		Images[1] = show;
		Mat combine = combineImages(Images, 2, 1, false);  //��϶���ͼƬ
		imshow("�˹��ָ�", combine);//����һ��������ʾ��ͼ����
		//imshow("�˹��ָ�", SRC);
		SRC.copyTo(DST);
	}
}

Mat Interaction::combineImages(vector<Mat> imgs,//@parameter1:��Ҫ��ʾ��ͼ���� 
	int col,//parameter2:��ʾ������
	int row, //parameter3:��ʾ������
	bool hasMargin)
{
	//parameter4:�Ƿ����ñ߿�
	int imgAmount = imgs.size();//��ȡ��Ҫ��ʾ��ͼ������
	int width = imgs[0].cols;//������Ĭ����Ҫ��ʾ��ͼ���С��ͬ
	int height = imgs[0].rows;//��ȡͼ����
	int newWidth, newHeight;//��ͼ����
	if (!hasMargin){
		newWidth = col*imgs[0].cols;//�ޱ߿���ͼ���/��=ԭͼ���/��*��/����
		newHeight = row*imgs[0].rows;
	}
	else{
		newWidth = (col + 1) * 20 + col*width;//�б߿�Ҫ���ϱ߿�ĳߴ磬�������ñ߿�Ϊ20px
		newHeight = (row + 1) * 20 + row*height;
	}

	Mat newImage(newHeight, newWidth, CV_8UC3, Scalar(255, 255, 255));//��ʾ�����趨�ߴ���µĴ�ͼ��ɫ���λ��ͨ�������Ϊ��ɫ
	//imshow("new", newImage);

	int x, y, imgCount;//x�кţ�y�кţ�imgCountͼƬ���
	if (hasMargin){//�б߿�
		imgCount = 0;
		x = 0; y = 0;
		while (imgCount < imgAmount){
			Mat imageROI = newImage(Rect(x*width + (x + 1) * 20, y*height + (y + 1) * 20, width, height));//��������Ȥ����
			imgs[imgCount].copyTo(imageROI);//��ͼ���Ƶ���ͼ��
			imgCount++;
			if (x == (col - 1)){
				x = 0;
				y++;
			}
			else{
				x++;
			}//�ƶ����кŵ���һ��λ��
		}
	}
	else{//�ޱ߿�
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
	return newImage;//�����µ����ͼ��
};

//�жϷ�����ײ\�ص�
bool Interaction::judge(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name)
{
	//cout << "pre" << pre_minellipse.size() << endl;
	//cout << "aft" << aft_minellipse.size() << endl;
	Mat img = imread(aft_name);   //��ȡԭͼ,Ϊ���ͼƬ�ߴ�
	int rows = img.rows;
	int cols = img.cols;
	struct part          //���ڴ洢ÿ��С������Ϣ�Ľṹ��
	{
		int num;     //ϸ����Ŀ
		vector<RotatedRect> minellipse;  //ϸ����Ϣ
	};
	vector<part> pre_part(9);
	vector<part> aft_part(9);
	vector<RotatedRect>::iterator it;

	vector<part>::iterator part_it;  //��ʼ��
	for (part_it = pre_part.begin(); part_it != pre_part.end(); part_it++)
	{
		part_it->num = 0;
	}
	for (part_it = aft_part.begin(); part_it != aft_part.end(); part_it++)
	{
		part_it->num = 0;
	}
	//cout << "pre  size:" << pre_minellipse.size() << endl;

	//���򻮷��������
	// [0 3 6]
	// [1 4 7]
	// [2 5 8]
	//ǰ֡pre���ͳ��
	for (it = pre_minellipse.begin(); it != pre_minellipse.end(); it++)
	{
		if ( it->center.x <= (cols / 3) )              //��һ��
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
		else if ( it->center.x <= (cols / 3 * 2) )          //�ڶ���
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
		else                                           //������
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

	//��֡aft���ͳ��
	for (it = aft_minellipse.begin(); it != aft_minellipse.end(); it++)
	{
		if (it->center.x <= (cols / 3))              //��һ��
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
		else if (it->center.x <= (cols / 3 * 2))          //�ڶ���
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
		else                                           //������
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

	HungarianAlgorithm HungAlgo;  //�ֲ��������㷨ƥ�����
	
	//�жϷ����ص�/��ײ������
	//ѭ��������оֲ����������һ���������������򷵻�true
	for (int k = 0; k < 9; k++)    
	{
		if ( (pre_part[k].num > aft_part[k].num) && (1) )  //�ֲ�������Ŀ���٣�����ԭ��1����ײ 2�����߽� 3��û�дӱ����м��ָ
		{
			//С����ֲ�������ƥ��
			//��ʼ��Ȩֵ����
			int pre_num = pre_part[k].minellipse.size();
			int aft_num = aft_part[k].minellipse.size();
			vector<double> temp;  //��ʱ��������ʼ��ʹ��
			temp.resize(aft_num);
			vector<vector<double>> connect;    //Ȩֵ����
			connect.resize(pre_num, temp);
			vector<int> matching;
			for (int a = 0; a < pre_num; a++)    //��ʼ����Ҫ��Ƹ���������Ȩ�س�����
			{
				for (int b = 0; b < aft_num; b++)
				{
					connect[a][b] = sqrt(pow(pre_part[k].minellipse[a].center.y - aft_part[k].minellipse[b].center.y, 2) + pow(pre_part[k].minellipse[a].center.x - aft_part[k].minellipse[b].center.x, 2));
				}
			}
			HungAlgo.Solve(connect, matching);   //Hungarian ƥ��
			//cout << endl;
			//cout << "pre:" << pre_part[k].num << "  ";
			//cout << "matching:";
			for (int i = 0; i < matching.size(); i++)
			{
				//*********************************************************************
				//������ֵ��������Ȩֵ�ϴ���������ӵ�ǿ�ƶϿ�,�õ�reliable��tracklets
				//*********************************************************************
				if (matching[i] != -1 && connect[i][matching[i]] > REMOTE)
				{
					matching[i] = -1;
				}

				//cout << matching[i] << ",";

				//�жϾֲ������������ͻ��
				if ((matching[i] != -1) && ((aft_part[k].minellipse[matching[i]].size.height * aft_part[k].minellipse[matching[i]].size.width) / (pre_part[k].minellipse[i].size.height * pre_part[k].minellipse[i].size.width) > THR_AREA) )
				{
					cout << "����" << (k + 1) << endl;  //������������ĵ�һ���ֲ�����
					return true;
				}
			 }
			//cout << "����" << (k + 1) << endl;  //������������ĵ�һ���ֲ�����
			//return true;
		}


	}


	return false;   //�����оֲ�������������������򷵻�false��
}

vector<RotatedRect> Interaction::Solve(vector<RotatedRect> pre_minellipse, vector<RotatedRect> aft_minellipse, string aft_name)
{
	Mat show;
	vector<RotatedRect> temp_minellipse;
	segmentation SegAlgo;

	if (judge(pre_minellipse, aft_minellipse, aft_name) )   //����Ƿ����ص�����ײ������
	{
		I_img = imread(aft_name);   //��ȡԭͼ
		if (I_img.empty())
		{
			//����Ƿ��ȡͼ��
			cout << "error! ͼƬ��ȡʧ��\n";
			waitKey(0);
		}
		else
		{
			namedWindow("�˹��ָ�", WINDOW_NORMAL);  //������Ҫ�޸Ĵ��ڴ�С������ѡ��WINDOW_NORMAL������WINDOW_AUTOSIZE
			SRC = I_img;
			Images[0] = SRC;
			//imshow("�˹��ָ�", SRC);

			cvtColor(I_img, I_img_gray, COLOR_BGR2GRAY);   //ת��Ϊ�Ҷ�ͼ
			show = SegAlgo.segment(aft_name, I_img_gray, I_img_dst, temp_minellipse);    //��ֵ�ָ�
			Images[1] = show;
			Mat combine = combineImages(Images, 2, 1, false);  //��϶���ͼƬ
			imshow("�˹��ָ�", combine);//����һ��������ʾ��ͼ����
			//imshow("�Ҷ�", show);

			SRC.copyTo(DST);
			setMouseCallback("�˹��ָ�", Interaction::linemouse, this);
			waitKey(0);
		}
		
		cout << "��ײ/�ص���" << aft_name << endl;    //��ӡ��ײ�ص�֡
		//imshow("last", DST);  //��ʾ��һ�λ��ֺ��ԭͼ
		vector<RotatedRect> dst_minellipse;
		cvtColor(DST, DST_gray, COLOR_BGR2GRAY);   //ת��Ϊ�Ҷ�ͼ
		SegAlgo.segment(aft_name, DST_gray, DST_dst, dst_minellipse);    //��ֵ�ָ�
		//cout << "~Interaction aft size:" << aft_minellipse.size() << endl;
		//cout << "Interaction dst size:" << dst_minellipse.size() << endl;
		return dst_minellipse;
	}
	
	return aft_minellipse;
}