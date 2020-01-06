#include "MatPlot.h"
#include <Python.h>

MatPlot::MatPlot(){}
MatPlot::~MatPlot(){}

void MatPlot::Plot_trajectory()
{
	//调用Python，利用库matplotlib画图
	Py_Initialize(); /*初始化python解释器,告诉编译器要用的python编译器*/
	PyRun_SimpleString("import matplotlib.pyplot as plt"); /*调用python文件*/
	PyRun_SimpleString("plt.plot([1,2,3,4], [12,3,23,231])"); /*调用python文件*/
	PyRun_SimpleString("plt.show()"); /*调用python文件*/
	Py_Finalize(); /*结束python解释器，释放资源*/
}