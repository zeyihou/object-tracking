#include "tracker.h"

#define IM_NUM 50 //����ͼƬ������
#define REMOTE 20   //����֮֡ǰƥ��ľ������ޣ����볬��REMOTE������й켣�ж�
#define TRACKLET_REMOTE 100    //����tracklet֮ǰƥ��ľ�������
//Ȩ�س�����
#define W_Euclidean 1
#define PI 3.1415926

#define INF 1000  //�����
#define LENGTH 50

tracker::tracker()
{
	minellipse.resize(IM_NUM);
	matching.resize(IM_NUM - 1);
	tracklets_sort.resize(IM_NUM - 1);
}
tracker::~tracker(){}

Mat tracker::read_trans(string path)    //��ȡͼƬת���ɻҶ�ͼ
{
	Mat img = imread(path);
	if (img.empty())
	{
		//����Ƿ��ȡͼ��
		cout << "error! ͼƬ��ȡʧ��\n";
		waitKey(0);
		getchar();
		return img;
	}
	else
	{
		cvtColor(img, img, COLOR_BGR2GRAY);   //ת��Ϊ�Ҷ�ͼ
		return img;
	}
}

void tracker::form_tracklets()
{
	string s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	string num;
	string s2 = ".jpg";
	trace_gray = read_trans("D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_0000.jpg");

	segmentation SEG;
	trace_draw = SEG.segment("trace", trace_gray, trace_dst, trace_minellipse);       //�洢��ʼ����Ϣ
	pre_minellipse = trace_minellipse;      //pre_minellipse Ϊÿ��ƥ���е� ǰ֡

	minellipse[0].obj_minellipse = trace_minellipse;
	minellipse[0].pre_link.resize(trace_minellipse.size());
	for (int n = 0; n < trace_minellipse.size(); n++)
	{
		minellipse[0].pre_link[n] = 1;
	}

	int pre_num, aft_num;
	HungarianAlgorithm HungAlgo;

	for (int i = 1; i < IM_NUM; i++)    //�ָ�����ͼƬ
	{
		if (i < 10)
			s1 += "000";
		else if (i<100)
			s1 += "00";
		else
			s1 += "0";

		num = to_string(i);
		s1 += num;
		s1 += s2;               //s1  Ϊ��ɵ�·����
		aft_gray = read_trans(s1);
		string aft_name = "aft" + num;
		SEG.segment(aft_name, aft_gray, aft_dst, aft_minellipse);    // aft_minellipse Ϊÿ��ƥ���е� ��֡

		minellipse[i].obj_minellipse = aft_minellipse;
		minellipse[i].pre_link.resize(aft_minellipse.size());
		for (int n = 0; n < aft_minellipse.size(); n++)
		{
			minellipse[i].pre_link[n] = 1;
		}

		//��ʼ��Ȩֵ����
		pre_num = pre_minellipse.size();
		aft_num = aft_minellipse.size();
		vector<double> temp;  //��ʱ��������ʼ��ʹ��
		temp.resize(aft_num);
		vector<vector<double>> connect;    //Ȩֵ����
		connect.resize(pre_num, temp);
		for (int a = 0; a < pre_num; a++)    //��ʼ����Ҫ��Ƹ���������Ȩ�س�����
		{
			for (int b = 0; b < aft_num; b++)
			{
				connect[a][b] = W_Euclidean * sqrt(pow(pre_minellipse[a].center.y - aft_minellipse[b].center.y, 2) + pow(pre_minellipse[a].center.x - aft_minellipse[b].center.x, 2))
					//+	 PI * abs((pre_minellipse[a].size.height / 2) *(pre_minellipse[a].size.width / 2) - (aft_minellipse[b].size.height / 2)*(aft_minellipse[b].size.height / 2) )
					;
			}
		}

		//Hungarian ƥ��
		HungAlgo.Solve(connect, matching[i - 1]);

		//*********************************************************************
		//������ֵ��������Ȩֵ�ϴ���������ӵ�ǿ�ƶϿ�,�õ�reliable��tracklets
		//*********************************************************************
		//
		for (int k = 0; k < connect.size(); k++)
		{
			int j = matching[i - 1][k];
			if (j != -1 && connect[k][j] > REMOTE)         //ע��: Assignment[i] ��ֵ����ȡ�� -1 ��ʹ���±�Խ��
			{
				matching[i - 1][k] = -1;              //�жϵõ�reliable��tracklets
			}

		}

		//��ǹ켣�ο�ʼλ�ã������� i-1 ֡�� i ֮֡���ƥ���ϵ��
		for (int n = 0; n < matching[i - 1].size(); n++)
		{
			if (matching[i - 1][n] != -1)
			{
				minellipse[i].pre_link[matching[i - 1][n]] = 0;
			}
		}

		Scalar color = Scalar(0, 255, 0);   //�켣��ɫ
		for (int k = 0; k < matching[i - 1].size(); k++)             //���� matching[] �������
		{
			//cout << matching[i-1][k] << endl;
			if (matching[i - 1][k] != -1)
			{
				line(trace_draw, pre_minellipse[k].center, aft_minellipse[matching[i - 1][k]].center, color, 1, 8);
			}
		}

		pre_minellipse = aft_minellipse;      //��֡��Ϣ��ֵ��ǰ֡
		s1 = "D:\\Mot VS2013 project\\Mot test1\\video_dataset\\sperm_";
	}

	namedWindow("trace", WINDOW_AUTOSIZE);
	imshow("trace", trace_draw);
	return;
}

void tracker::form_trajectory()
{
	for (int i = 1; i < IM_NUM; i++)
	{
		//��ʱ������matching[i-1]���㵱ǰ��֮֡���tracklets
		for (int m = 0; m < matching[i - 1].size(); m++)
		{
			if ((minellipse[i - 1].pre_link[m] == 1) && (matching[i - 1][m] != -1))    //��tracklets�� ��ʼ�� (������֮ǰδƥ������һ�ƥ����һ��)
			{
				int sum = 0;    //����������������켣���Ӵ���
				tracklet tem;
				tem.t_s = i - 1;
				tem.s_ID = m;

				int cur = matching[i - 1][m];     //ѭ��
				tem.t_e = i - 1;
				int k = m;
				//int x = i - 1;
				while (cur != -1)
				{
					sum++;
					tem.t_e += 1;
					tem.e_ID = cur;

					tem.len += sqrt(pow(minellipse[tem.t_e - 1].obj_minellipse[k].center.y - minellipse[tem.t_e].obj_minellipse[cur].center.y, 2) + pow(minellipse[tem.t_e - 1].obj_minellipse[k].center.x - minellipse[tem.t_e].obj_minellipse[cur].center.x, 2));
					k = matching[tem.t_e - 1][k];
					if (tem.t_e > IM_NUM - 2)
					{
						//cout << "matchingԽ��" << endl;
						break;
					}
					cur = matching[tem.t_e][cur];
				}
				tem.v = tem.len / sum;
				tracklets_sort[i - 1].push_back(tem);
				tracklets.push_back(tem);  //���һ���켣
			}

		}
	}

	//******************************
	//tracklets֮�����hungarian
	//******************************
	vector<double> temp;  //��ʱ��������ʼ��ʹ��
	temp.resize(tracklets.size());
	vector<vector<double>> track_connect;   //Ȩֵ����
	track_connect.resize(tracklets.size(), temp);

	//const int INF = 1000;  //�����
	//const int LENGTH = 50;

	for (int i = 0; i < tracklets.size(); i++)      //ÿһ��tracklets֮������������
	{
		for (int j = 0; j < tracklets.size(); j++)
		{
			RotatedRect pre = minellipse[tracklets[i].t_e].obj_minellipse[tracklets[i].e_ID];
			RotatedRect aft = minellipse[tracklets[j].t_s].obj_minellipse[tracklets[j].s_ID];

			if (sqrt(pow(pre.center.y - aft.center.y, 2) + pow(pre.center.x - aft.center.x, 2))<LENGTH && (tracklets[i].t_e < tracklets[j].t_s) )
				//if ((tracklets[i].t_e < tracklets[j].t_s)  )             //����
			{
				track_connect[i][j] = abs(tracklets[i].v - tracklets[j].v);
			}
			else
			{
				track_connect[i][j] = INF;     //Ϊ�����ܵ�trackletsƥ�䣬����һ���ϴ��Ȩֵ
			}
		}
	}

	//Hungarian ƥ��
	HungarianAlgorithm HungAlgo;
	//vector<int> track_matching(tracklets.size() - 1);    //track_matching
	track_matching.resize(tracklets.size() - 1);
	HungAlgo.Solve(track_connect, track_matching);


	for (int i = 0; i < track_matching.size(); i++)   //ɸѡ
	{
		if (track_connect[i][track_matching[i]] == INF)
		{
			track_matching[i] = -1;                                  //�Ͽ�
		}
	}

	Scalar color = Scalar(0, 0, 255);   //�켣��ɫ
	for (int k = 0; k < track_matching.size(); k++)             //���� track_matching[] �������
	{
		if (track_matching[k] != -1)
		{
			line(trace_draw, minellipse[tracklets[k].t_e].obj_minellipse[tracklets[k].e_ID].center, minellipse[tracklets[track_matching[k]].t_s].obj_minellipse[tracklets[track_matching[k]].s_ID].center, color, 1, 8);
		}
	}

	//******************************************************
	//��������trajectory�еĵ���ÿһʱ�̵�λ�ã����ڻ�ͼչʾ
	//�����¼��motion_position�ṹ��
	//******************************************************

	vector<bool> aft_mark; //���ڱ��ÿһ���켣��tracklet�Ƿ��Ѿ�����ĳһ��trajectory,�Ƿ����ƥ��, (  aft_mark[] ��Ӧ tracklets[]  )
	aft_mark.resize(tracklets.size(),true);        //��ʼ��,������ƥ��
	
	int time, ID;

	for (int i = 0; i < tracklets.size(); i++)
	{
		vector<TimePosition> time_position;

		if (aft_mark[i] == true)          //����һ���켣��tracklet�����û�й���
		{
			TimePosition tp;
			time = tracklets[i].t_s;
			ID = tracklets[i].s_ID;
			tp.time = time;
			tp.position = minellipse[time].obj_minellipse[ID].center;
			time_position.push_back(tp);
			while (matching[time][ID] != -1)
			{
				ID = matching[time][ID];
				++time;
				tp.time = time;
				tp.position = minellipse[time].obj_minellipse[ID].center;
				time_position.push_back(tp);
				if (time > (matching.size()-1) )
				{
					break;
				}
			}
			aft_mark[i] = false;      //ͳ����һ���켣

			int k = i;
			while (track_matching[k] != -1)
			{
				k = track_matching[k];
				time = tracklets[k].t_s;
				ID = tracklets[k].s_ID;
				tp.time = time;
				tp.position = minellipse[time].obj_minellipse[ID].center;
				time_position.push_back(tp);
				while (matching[time][ID] != -1)
				{
					ID = matching[time][ID];
					++time;
					tp.time = time;
					tp.position = minellipse[time].obj_minellipse[ID].center;
					time_position.push_back(tp);
					if (time > (matching.size() - 1))
					{
						break;
					}
				}
				aft_mark[k] = false;      //ͳ����һ���켣
			}

			motion_location.push_back(time_position);
		}
	}

	namedWindow("trace", WINDOW_AUTOSIZE);
	imshow("trace", trace_draw);

	return;

}

