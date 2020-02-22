//**********************************************************//
//                  �˻�Э��         
//         Human-Machine Collaboration
//        �ж�ǰ����֡Ŀ����ص�����ײ 
//*********************************************************//

#include "Interaction.h"
#include "segmentation.h"

#include <opencv2/opencv.hpp>             //�����Ӧ
#include <opencv2/opencv_modules.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>

Interaction::Interaction(){}
Interaction::~Interaction(){}

void line_mouse(int event, int x, int y, int flags, void* ustc);
Mat combineImages(vector<Mat> imgs, int col, int row, bool hasMargin);

Mat SRC, DST, DST_gray, DST_dst;  //��껮���и��ԭͼ��Ŀ��ͼ
Mat I_img, I_img_gray, I_img_dst, show;   //���ڴ洢ͼƬ
vector<Mat> Images(2);//ģ����vector�����ڷ���2������ΪMat��Ԫ�أ���2��ͼƬ 

void line_mouse(int event, int x, int y, int flags, void* ustc)     //��껮����Ӧ����
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

Mat combineImages(vector<Mat> imgs,//@parameter1:��Ҫ��ʾ��ͼ���� 
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

	if ( judge(pre_minellipse, aft_minellipse) )   //����Ƿ����ص�����ײ������
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
			setMouseCallback("�˹��ָ�", line_mouse, 0);
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