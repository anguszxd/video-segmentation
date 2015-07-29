// ConsoleApplication3.cpp : 定义控制台应用程序的入口点。
//


// gmm_wavetrees.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "opencv2/core/core.hpp"
#include "opencv2/video/background_segm.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <stdio.h>

using namespace std;
using namespace cv;

//this is a sample for foreground detection functions
string src_img_name = "WavingTrees/b00";
const char *src_img_name1;
Mat img, fgmask, fgimg;
int i = -1;
char chari[500];
bool update_bg_model = true;
bool pause = false;

//第一种gmm,用的是KaewTraKulPong, P. and R. Bowden (2001).
//An improved adaptive background mixture model for real-time tracking with shadow detection.
BackgroundSubtractorMOG bg_model;

void refineSegments(const Mat& img, Mat& mask, Mat& dst)
{
	int niters = 3;

	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;

	Mat temp;

	dilate(mask, temp, Mat(), Point(-1, -1), niters);//膨胀，3*3的element，迭代次数为niters
	erode(temp, temp, Mat(), Point(-1, -1), niters * 2);//腐蚀
	dilate(temp, temp, Mat(), Point(-1, -1), niters);

	findContours(temp, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);//找轮廓

	dst = Mat::zeros(img.size(), CV_8UC3);

	if (contours.size() == 0)
		return;

	// iterate through all the top-level contours,
	// draw each connected component with its own random color
	int idx = 0, largestComp = 0;
	double maxArea = 0;

	for (; idx >= 0; idx = hierarchy[idx][0])//这句没怎么看懂
	{
		const vector<Point>& c = contours[idx];
		double area = fabs(contourArea(Mat(c)));
		if (area > maxArea)
		{
			maxArea = area;
			largestComp = idx;//找出包含面积最大的轮廓
		}
	}
	Scalar color(0, 255, 0);
	drawContours(dst, contours, largestComp, color, CV_FILLED, 8, hierarchy);
}

int main(int argc, const char** argv)
{
	//bg_model.noiseSigma = 10;
	img = imread("WavingTrees/b00000.bmp");
	if (img.empty())
	{
		namedWindow("image", 1);//不能更改窗口
		namedWindow("foreground image", 1);
		namedWindow("mean background image", 1);
	}
	for (;;)
	{
		if (!pause)
		{
			i++;
			itoa(i, chari, 10);
			if (i<10)
			{
				src_img_name += "00";
			}
			else if (i<100)
			{
				src_img_name += "0";
			}
			else if (i>285)
			{
				i = -1;
			}
			if (i >= 230)
				update_bg_model = false;
			else update_bg_model = true;

			src_img_name += chari;
			src_img_name += ".bmp";

			img = imread(src_img_name);
			if (img.empty())
				break;

			//update the model
			bg_model(img, fgmask, update_bg_model ? 0.005 : 0);//计算前景mask图像，其中输出fgmask为8-bit二进制图像，第3个参数为学习速率
			refineSegments(img, fgmask, fgimg);

			imshow("image", img);
			imshow("foreground image", fgimg);

			src_img_name = "WavingTrees/b00";

		}
		char k = (char)waitKey(80);
		if (k == 27) break;

		if (k == ' ')
		{
			pause = !pause;
		}
	}

	return 0;
}