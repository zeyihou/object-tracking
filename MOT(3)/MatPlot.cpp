#include "MatPlot.h"
#include <Python.h>

MatPlot::MatPlot(){}
MatPlot::~MatPlot(){}

void MatPlot::Plot_trajectory_3D(vector<vector<TimePosition>> plot_data)
{
	//cout << plot_data[10][20].time<<endl;
	//cout << plot_data[10][20].position.x << endl;

	//����python�ű�
	Py_Initialize(); /*��ʼ��python������,���߱�����Ҫ�õ�python������*/
	if (!Py_IsInitialized())
	{
		cout << "��ʼ��ʧ�ܣ�" << endl;
		Py_Finalize();
	}
	PyObject * pModule = NULL;
	PyObject * pFunc = NULL;

	PyRun_SimpleString("import sys");     //���Insertģ��·��  
	PyRun_SimpleString("sys.path.append('./')");
	pModule = PyImport_ImportModule("MyPython");      //Python�ļ���  

	if (!pModule) // ����ģ��ʧ��
	{
		cout << "[ERROR] py�ļ�����ʧ�ܣ� Python get module failed." << endl;
		return;
	}
	pFunc = PyObject_GetAttrString(pModule, "plot_3d");  //Python�ļ��еĺ�����
	if (!pModule) // ����ģ��ʧ��
	{
		cout << "[ERROR] ��������ʧ�ܣ� Python get function failed." << endl;
		return;
	}
	 
	//��������:  
	PyObject* Args_X = PyTuple_New(plot_data.size());    //��άԪ��
	PyObject* Args_Y = PyTuple_New(plot_data.size());
	PyObject* Args_T = PyTuple_New(plot_data.size());
	for (int i = 0; i < plot_data.size(); i++)
	{
		PyObject* temp_X = PyTuple_New(plot_data[i].size());     //Ԫ��
		PyObject* temp_Y = PyTuple_New(plot_data[i].size());
		PyObject* temp_T = PyTuple_New(plot_data[i].size());
		for (int j = 0; j < plot_data[i].size(); j++)
		{
			PyTuple_SetItem(temp_X, j, Py_BuildValue("d", plot_data[i][j].position.x));      //Py_BuildValue��c++���ݷ�װ��python����
			PyTuple_SetItem(temp_Y, j, Py_BuildValue("d", plot_data[i][j].position.y));
			PyTuple_SetItem(temp_T, j, Py_BuildValue("i", plot_data[i][j].time));
		}
		PyTuple_SetItem(Args_X, i, temp_X);
		PyTuple_SetItem(Args_Y, i, temp_Y);
		PyTuple_SetItem(Args_T, i, temp_T);
	}

	PyObject* Args = PyTuple_New(3);  //����python������������װ��һ��Ԫ�棬��Ӧpython�к�����Ҫ�����������Ԫ��Ԫ����Ŀ��ͬ������SystemError��
	PyTuple_SetItem(Args, 0, Args_X);
	PyTuple_SetItem(Args, 1, Args_Y);
	PyTuple_SetItem(Args, 2, Args_T);

	PyObject* result = NULL;
	result = PyEval_CallObject(pFunc, Args);    //���ú���
	if (NULL == result)
	{
		cout << "����python����ʧ��" << endl;
	}

	












	//PyObject* pyParams = Py_BuildValue("(s)", "helllo world!");//c++����תpython����
	//char * result1;
	//pArg = PyEval_CallObject(pFunc, pyParams);    //���ú���
	//PyArg_Parse(pArg, "s", &result1);     //python����תc++����
	//cout << result1 << endl;




	


	//test##############################
	//cout << plot_data[0][0].position.x << endl;
	//PyObject* arg1 = Py_BuildValue("d", plot_data[0][0].position.x);
	//double w = PyFloat_AsDouble(arg1);
	//cout << w;
	//PyRun_SimpleString("");
	//����Python�����ÿ�matplotlib��ͼ
	//Py_Initialize(); /*��ʼ��python������,���߱�����Ҫ�õ�python������*/
	//PyRun_SimpleString("import matplotlib.pyplot as plt"); /*����python�ļ�*/
	//PyRun_SimpleString("import numpy as np");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d import Axes3D");
	//PyRun_SimpleString("import random");
	//PyRun_SimpleString("import matplotlib as mpl");
	//PyRun_SimpleString("from matplotlib import cm");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d import axes3d");
	//PyRun_SimpleString("from mpl_toolkits.mplot3d.art3d import Poly3DCollection, Line3DCollection");
	//PyRun_SimpleString("x = np.linspace(1, 20, 20)");
	//PyRun_SimpleString("y = np.arange(10, 30, 1)");
	//PyRun_SimpleString("z = np.random.randint(20, 50, 20)  # numpy�ֱ���������ά������");
	//PyRun_SimpleString("fig = plt.figure()");
	//PyRun_SimpleString("ax = Axes3D(fig)");
	//PyRun_SimpleString("ax.plot(x, y, z, 'bo--')");
	//PyRun_SimpleString("plt.show()"); /*����python�ļ�*/

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
	//	z = np.random.randint(20, 50, 20)  # numpy�ֱ���������ά������
	//	fig = plt.figure()
	//	ax = Axes3D(fig)
	//	ax.plot(x, y, z, 'bo--')
	//	plt.show()
	//	return

	Py_Finalize(); /*����python���������ͷ���Դ*/


}