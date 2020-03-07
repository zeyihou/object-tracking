#include "segmentation.h"

#define thr 46  //��ʼ����ֵ  
#define IM_SIZE 3   //�ָ�ʱ��ɸѡ�ߴ�
//#define IM_ROUNDNESS 0.4   //�ָ�ʱ��ɸѡԲ��

segmentation::segmentation(){}
segmentation::~segmentation(){}

Mat segmentation::segment(string im_name, Mat &im_gray, Mat &im_dst, vector<RotatedRect> &im_minEllipse )
{
	//hierarchy��һ��������������ÿ��Ԫ�ض���һ������4��int�͵����顣����hierarchy�ڵ�Ԫ�غ���������contours�ڵ�Ԫ����һһ��Ӧ�ģ�������������ͬ��
	//hierarchy��ÿ��Ԫ�ص�4��int�ͱ�����hierarchy[i][0] ~hierarchy[i][3]���ֱ��
	//ʾ��ǰ���� i �ĺ�һ��������ǰһ������������������Ƕ�����ı������
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;
	//vector<double> im_roundness;

	//cout << "##################  threshold:   " << thr << "    #########################" << endl;
	threshold(im_gray, im_dst, thr, 255, THRESH_BINARY);  //��ֵ������ͼ��ָ�  THRESH_TOZERO\THRESH_BINARY\.....
	findContours(im_dst, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));   //������,Point��ƫ����

	//����������һЩԤ�������
	vector<vector<Point>>::iterator contours_it;     //����ָ��
	for (contours_it = contours.begin(); contours_it != contours.end();)    ////ɾȥС��10�����ص�����
	{
		if ((*contours_it).size() < 6)      //ɾȥС��10�����صı�Ե
		{
			contours_it = contours.erase(contours_it);    //��ɾ���ɹ�����ָ���Ѿ�+1
		}
		else
		{
			++contours_it;
		}
	} 
	 
	//im_roundness.resize(contours.size());    //  ÿ��������Բ�ζ�
	im_minEllipse.resize(contours.size());    //RotatedRect��һ���洢��ת���ε��࣬���ڴ洢��Բ��Ϻ������صĽ��

	for (int i = 0; i < contours.size(); i++)   //��ÿһ����������С��ΧԲ
	{
		//��ָ���ĵ㼯���ж���αƽ��ĺ���,�����⻬�������߻�����ͼ����������ж������ϡ�
		//approxPolyDP(contours[i], contours[i], 1, true);    //true���������Ǳպϵ�
		if (contours[i].size() > 5)          //����6���㻭��Բ
		{
			im_minEllipse[i] = fitEllipse(contours[i]);    //��С�����Բ
		//	im_roundness[i] = (4 * CV_PI * contourArea(contours[i])) / (arcLength(contours[i], true)  *arcLength(contours[i], true));//Բ��
		}
		else
			cout << "���ص�����<5" << endl;
	}

	//..������ɸѡ��������������������������������������������
	//vector<double>::iterator roundness_it = im_roundness.begin();    //  Բ�ζ�ָ��
	vector<RotatedRect>::iterator minEllipse_it = im_minEllipse.begin();    //��Բָ��
	for (contours_it = contours.begin(); contours_it != contours.end();)    //it++Ҫд��for���棬������ɾ������ֵ
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

	//��ͼ
	Mat drawing = Mat::zeros(im_dst.size(), CV_8UC3);   //�ػ�һ����ͨ���ģ�����Ϊ��ɫ��ͼ
	for (int i = 0; i< contours.size(); i++)
	{
		Scalar color_R = Scalar(0, 255, 255);
		Scalar color = Scalar(255, 0, 0);
		drawContours(drawing, contours, i, color_R, 1, 8, vector<Vec4i>(), 0, Point());
		ellipse(drawing, im_minEllipse[i], color, 1, 8);   //�����Բ����¼ÿ��ϸ����Ϣ
	//	cout << (i + 1) << im_minEllipse[i].center << "   minor axis:  " << setprecision(6) << im_minEllipse[i].size.width << "  long axis:  " << setprecision(6) << im_minEllipse[i].size.height << "  roundness:" << setprecision(6) << im_roundness[i] << endl;
	}
	//cout << endl << endl;
	/// �ڴ�������ʾ���
	//namedWindow(im_name, WINDOW_AUTOSIZE);
	//imshow(im_name, drawing);
	return drawing;
}
