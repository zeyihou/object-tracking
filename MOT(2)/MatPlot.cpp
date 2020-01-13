#include "MatPlot.h"
#include <Python.h>

MatPlot::MatPlot(){}
MatPlot::~MatPlot(){}

void MatPlot::Plot_trajectory_3D(vector<vector<TimePosition>> plot_data)
{
	//cout << plot_data[10][20].time<<endl;
	//cout << plot_data[10][20].position.x << endl;

	//调用python脚本
	Py_Initialize(); /*初始化python解释器,告诉编译器要用的python编译器*/
	if (!Py_IsInitialized())
	{
		cout << "初始化失败！" << endl;
		Py_Finalize();
	}
	PyObject * pModule = NULL;
	PyObject * pFunc = NULL;

	PyRun_SimpleString("import sys");     //添加Insert模块路径  
	PyRun_SimpleString("sys.path.append('./')");
	pModule = PyImport_ImportModule("MyPython");      //Python文件名  

	if (!pModule) // 加载模块失败
	{
		cout << "[ERROR] py文件导入失败！ Python get module failed." << endl;
		return;
	}
	pFunc = PyObject_GetAttrString(pModule, "plot_3d");  //Python文件中的函数名
	if (!pModule) // 加载模块失败
	{
		cout << "[ERROR] 函数导入失败！ Python get function failed." << endl;
		return;
	}
	 
	//创建参数:  
	PyObject* Args_X = PyTuple_New(plot_data.size());    //二维元组
	PyObject* Args_Y = PyTuple_New(plot_data.size());
	PyObject* Args_T = PyTuple_New(plot_data.size());
	for (int i = 0; i < plot_data.size(); i++)
	{
		PyObject* temp_X = PyTuple_New(plot_data[i].size());     //元组
		PyObject* temp_Y = PyTuple_New(plot_data[i].size());
		PyObject* temp_T = PyTuple_New(plot_data[i].size());
		for (int j = 0; j < plot_data[i].size(); j++)
		{
			PyTuple_SetItem(temp_X, j, Py_BuildValue("d", plot_data[i][j].position.x));      //Py_BuildValue把c++数据封装成python数据
			PyTuple_SetItem(temp_Y, j, Py_BuildValue("d", plot_data[i][j].position.y));
			PyTuple_SetItem(temp_T, j, Py_BuildValue("i", plot_data[i][j].time));
		}
		PyTuple_SetItem(Args_X, i, temp_X);
		PyTuple_SetItem(Args_Y, i, temp_Y);
		PyTuple_SetItem(Args_T, i, temp_T);
	}

	PyObject* Args = PyTuple_New(3);  //传给python的三个参数封装成一个元祖，对应python中函数，要求参数个数和元祖元素数目相同，否则SystemError！
	PyTuple_SetItem(Args, 0, Args_X);
	PyTuple_SetItem(Args, 1, Args_Y);
	PyTuple_SetItem(Args, 2, Args_T);

	PyObject* result = NULL;
	result = PyEval_CallObject(pFunc, Args);    //调用函数
	if (NULL == result)
	{
		cout << "调用python函数失败" << endl;
	}

	












	//PyObject* pyParams = Py_BuildValue("(s)", "helllo world!");//c++类型转python类型
	//char * result1;
	//pArg = PyEval_CallObject(pFunc, pyParams);    //调用函数
	//PyArg_Parse(pArg, "s", &result1);     //python类型转c++类型
	//cout << result1 << endl;




	


	//test##############################
	//cout << plot_data[0][0].position.x << endl;
	//PyObject* arg1 = Py_BuildValue("d", plot_data[0][0].position.x);
	//double w = PyFloat_AsDouble(arg1);
	//cout << w;
	//PyRun_SimpleString("");
	//调用Python，利用库matplotlib画图
	//Py_Initialize(); /*初始化python解释器,告诉编译器要用的python编译器*/
	//PyRun_SimpleString("import matplotlib.pyplot as plt"); /*调用python文件*/
	//PyRun_SimpleString("import numpy as np");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d import Axes3D");
	//PyRun_SimpleString("import random");
	//PyRun_SimpleString("import matplotlib as mpl");
	//PyRun_SimpleString("from matplotlib import cm");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d import axes3d");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection");
	//PyRun_SimpleString("x = np.linspace(1, 20, 20)");
	//PyRun_SimpleString("y = np.arange(10, 30, 1)");
	//PyRun_SimpleString("z = np.random.randint(20, 50, 20)  # numpy分别生成三个维度数据");
	//PyRun_SimpleString("fig = plt.figure()");
	//PyRun_SimpleString("ax = Axes3D(fig)");
	//PyRun_SimpleString("ax.plot(x, y, z, 'bo--')");
	//PyRun_SimpleString("plt.show()"); /*调用python文件*/

	//import matplotlib.pyplot as plt
	//	import numpy as np
	//	from mpl_toolkits.mplot3d import Axes3D
	//	import random
	//	import matplotlib as mpl
	//	from matplotlib import cm
	//	from mpl_toolkits.mplot3d import axes3d
	//	from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection

	//	def plot_3d() :
	//	print("hello!")
	//	x = np.linspace(1, 20, 20)
	//	y = np.arange(10, 30, 1)
	//	z = np.random.randint(20, 50, 20)  # numpy分别生成三个维度数据
	//	fig = plt.figure()
	//	ax = Axes3D(fig)
	//	ax.plot(x, y, z, 'bo--')
	//	plt.show()
	//	return

	Py_Finalize(); /*结束python解释器，释放资源*/


}