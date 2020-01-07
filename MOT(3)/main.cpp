#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>
#include <Python.h>

#include "segmentation.h"
#include "Hungarian.h"
#include "tracker.h"
#include "MatPlot.h"

using namespace std;
using namespace cv;


int main()
{
	tracker trackerAlgo;
	trackerAlgo.form_tracklets();   //形成轨迹段tracklets
	trackerAlgo.form_trajectory();      //tracklets之间相连接
	//waitKey(0);

	MatPlot PlotAlgo;
	//PlotAlgo.Plot_trajectory_3D();
	
	return 0;
}

