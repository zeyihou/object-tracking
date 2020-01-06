#ifndef MATPLOT_H
#define MATPLOT_H

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <stdlib.h>

using namespace std;
using namespace cv;

class MatPlot
{
public:
	MatPlot();
	~MatPlot();
	void Plot_trajectory();
};

#endif
