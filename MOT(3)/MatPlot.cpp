#include "MatPlot.h"
#include <Python.h>

MatPlot::MatPlot(){}
MatPlot::~MatPlot(){}

void MatPlot::Plot_trajectory()
{
	//����Python�����ÿ�matplotlib��ͼ
	Py_Initialize(); /*��ʼ��python������,���߱�����Ҫ�õ�python������*/
	PyRun_SimpleString("import matplotlib.pyplot as plt"); /*����python�ļ�*/
	PyRun_SimpleString("plt.plot([1,2,3,4], [12,3,23,231])"); /*����python�ļ�*/
	PyRun_SimpleString("plt.show()"); /*����python�ļ�*/
	Py_Finalize(); /*����python���������ͷ���Դ*/
}