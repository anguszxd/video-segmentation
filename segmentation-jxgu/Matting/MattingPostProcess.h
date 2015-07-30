#ifndef _MATTING_POST_PROCESS_H_
#define _MATTING_POST_PROCESS_H_
#include <opencv2/opencv.hpp>
using namespace cv;
void GammaCorrection(Mat& src, float fGamma);
void FindContour(Mat & img, int block_th);
void FindGeometry(const Mat & img, const Mat & src, float rasie_hands_radius, float ratio_x, float ratio_y);
bool get_body_centroid(int &x, int &y);
bool raise_hand_left(int &x, int &y);
bool raise_hand_right(int &x, int &y);
void do_preprocess(const Mat& frame_mono,Mat& frame);
#endif