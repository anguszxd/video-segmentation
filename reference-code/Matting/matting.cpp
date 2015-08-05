// matting.cpp : 定義主控台應用程式的進入點。
//
#include "Gpuopt.h"
#include <stdio.h>
#define MATTING_C
#include "matting.h"
#include "MattingPostProcess.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <mutex>
#include <thread>
#include <chrono>
#include <stdio.h>
using namespace cv;
using namespace std;

#define USE_GPU 0
#define WRITETODISK 1
bool do_self_test = true;

const string input_path = "C:/chenyu/work/matting/image/save4/";
const string output_path = "C:/chenyu/work/matting/out/save4/";
string img_name = "save4";
const int frame_start = 0;//84,53 
const int frame_end = 669;//450,230


extern int body_top ;
extern int body_bottom ;
extern int body_left ;
extern int body_right ;

int alpha_top = 40;
int alpha_bottom = 40;
int alpha_left = 120;
int alpha_right = 120;
float scale = 1.0f;

#ifdef DEBUG_DZH
int delay = 0;
float frames, frame_avg;
double start2, preve, camera_recv_buf,camera_recv;
#endif

Matting * matting;

struct TuningPara init_value = {
	0,
	32,
	32,
	32,
	16,
	32,
	4,
	4,
	4,
	32,
	7,
	500,
	1.2f,
	0.25f,
	0.0f,
	0.7f,
	1.0f
};

bool read_tune_para(string paramfile, TuningPara & tune_para)
{
	FileStorage fs(paramfile, FileStorage::READ);
	if (fs.isOpened()) {
		fs["show_content"] >> tune_para.show_content;
		fs["motion_TH"] >> tune_para.motion_TH;
		fs["alpha_TH"] >> tune_para.alpha_TH;
		fs["static_MAX"] >> tune_para.static_MAX;
		fs["init_static_num"] >> tune_para.init_static_num;
		fs["luma_offset"] >> tune_para.luma_offset;
		fs["u_gain"] >> tune_para.u_gain;
		fs["v_gain"] >> tune_para.v_gain;
		fs["static_NUM"] >> tune_para.static_NUM;
		fs["long_static_NUM"] >> tune_para.long_static_NUM;
		fs["alpha_filter_r"] >> tune_para.alpha_filter_r;
		fs["block_th"] >> tune_para.block_th;
		fs["rasie_hands_radius"] >> tune_para.rasie_hands_radius;
		fs["static_SPEED"] >> tune_para.static_SPEED;
		fs["long_SPEED"] >> tune_para.long_SPEED;
		fs["GAMMA"] >> tune_para.GAMMA;
		fs["process_Scale"] >> tune_para.process_Scale;
		fs.release();
		return true;
	}
	return false;
}

void write_tune_para(string paramfile, TuningPara & tune_para)
{
	FileStorage fs(paramfile, FileStorage::WRITE);
	fs << "show_content" << tune_para.show_content;
	fs << "motion_TH" << tune_para.motion_TH;
	fs << "alpha_TH" << tune_para.alpha_TH;
	fs << "static_MAX" << tune_para.static_MAX;
	fs << "init_static_num" << tune_para.init_static_num;
	fs << "luma_offset" << tune_para.luma_offset;
	fs << "u_gain" << tune_para.u_gain;
	fs << "v_gain" << tune_para.v_gain;
	fs << "static_NUM" << tune_para.static_NUM;
	fs << "long_static_NUM" << tune_para.long_static_NUM;
	fs << "alpha_filter_r" << tune_para.alpha_filter_r;
	fs << "block_th" << tune_para.block_th;
	fs << "rasie_hands_radius" << tune_para.rasie_hands_radius;
	fs << "static_SPEED" << tune_para.static_SPEED;
	fs << "long_SPEED" << tune_para.long_SPEED;
	fs << "GAMMA" << tune_para.GAMMA;
	fs << "process_Scale" << tune_para.process_Scale;
	fs.release();
}

bool operator== (TuningPara & t1, TuningPara & t2)
{
	return (
		t1.alpha_TH == t2.alpha_TH &&
		fabs(t1.GAMMA - t2.GAMMA) < 0.0001 &&
		t1.init_static_num == t2.init_static_num &&
		fabs(t1.long_SPEED - t2.long_SPEED) < 0.0001 &&
		t1.long_static_NUM == t2.long_static_NUM &&
		t1.luma_offset == t2.luma_offset &&
		t1.motion_TH == t2.motion_TH &&
		t1.show_content == t2.show_content &&
		t1.static_MAX == t2.static_MAX &&
		t1.static_NUM == t2.static_NUM &&
		t1.alpha_filter_r == t2.alpha_filter_r &&
		t1.block_th == t2.block_th &&
		fabs(t1.rasie_hands_radius - t2.rasie_hands_radius) < 0.0001 &&
		fabs(t1.static_SPEED - t2.static_SPEED) < 0.0001 &&
		t1.u_gain == t2.u_gain &&
		t1.v_gain == t2.v_gain &&
		fabs(t1.process_Scale - t2.process_Scale) < 0.0001);
}

void transform_para (TuningParaFloat &t1, TuningPara &t2)
{	
	t1.alpha_filter_r = t2.alpha_filter_r;
	t1.alpha_TH_f = t2.alpha_TH / 255.0f;
	t1.GAMMA = t2.GAMMA;
	t1.init_static_num = t2.init_static_num;
	t1.long_SPEED_f = t2.long_SPEED / 255.0f;
	t1.long_static_NUM = t2.long_static_NUM;
	t1.luma_offset_f = t2.luma_offset / 255.0f;
	t1.motion_TH_f = t2.motion_TH / 255.0f;
	t1.show_content = t2.show_content;
	t1.static_MAX = t2.static_MAX;
	t1.static_NUM = t2.static_NUM;
	t1.block_th = t2.block_th;
	t1.rasie_hands_radius = t2.rasie_hands_radius;
	t1.static_SPEED_f = t2.static_SPEED / 255.0f;
	t1.u_gain_f = (float)t2.u_gain;
	t1.v_gain_f = (float)t2.v_gain;
	t1.process_Scale = t2.process_Scale;
}

void check_tune_para_thread()
{
	TuningPara tune_para, old_tune_para;

	LOG(DBG_LEVEL, "check tune parameter thread start");
	old_tune_para = init_value;
	matting->tune_parameter(init_value);
	write_tune_para("TuningOut.xml", init_value);
	while (1) {
		LOG(DBG_LEVEL, "read tune parameter");
		if (read_tune_para("TuningIn.xml", tune_para)) {			
			write_tune_para("TuningOut.xml", tune_para);
			LOG(DBG_LEVEL, "write tune parameter");
			if (!(old_tune_para == tune_para)) {
				old_tune_para = tune_para;
				LOG(DBG_LEVEL, "tune parameter change, update");
				matting->tune_parameter(tune_para);
			}
		}
		chrono::milliseconds dura(500);
		this_thread::sleep_for(dura);
	}
}

MattingLog::MattingLog(string _filename, int _print_level, bool _log_time)
{
	start_time = (double)getTickCount();
	filename = _filename;
	print_level = _print_level;
	log_time = _log_time;
	fopen_s(&log_fp, _filename.c_str(), "w");
	fclose(log_fp);
	log_len = 0;
	log_buf = (char *)malloc(LOG_BUFFER);
}
MattingLog::~MattingLog()
{
	flush();
	free(log_buf);
}
double MattingLog::get_time()
{
	return ((double)getTickCount() - start_time) / getTickFrequency();
}

void MattingLog::log(char * s, int len)
{
	if (len + log_len +10 >= LOG_BUFFER)
		return;
	lock.lock();
	if (log_time)
		log_len += sprintf_s(&log_buf[log_len], LOG_BUFFER-log_len, "%9.4f:", get_time());
	memcpy(&log_buf[log_len], s, len);
	log_len += len;
	lock.unlock();
}

void MattingLog::flush()
{
	if (log_len == 0)
		return;
	fopen_s(&log_fp, filename.c_str(), "a");
	lock.lock();
	fwrite(log_buf, 1, log_len, log_fp);
	log_len = 0;
	lock.unlock();
	fclose(log_fp);
}

void log_thread()
{
	while (1) {
		matting_log->flush();
		chrono::milliseconds dura(1000);
		this_thread::sleep_for(dura);
	}
}

void init_log()
{
	matting_log = new MattingLog();
	new thread(log_thread);
}

static void sum_channel(const Mat & rgb, Mat &mono)
{
	switch (rgb.depth()) {
	case CV_8U:
		mono.create(rgb.rows, rgb.cols, CV_16U);
		for (int y = 0; y < rgb.rows; y++) {
			const unsigned char * pa = rgb.ptr<unsigned char>(y);
			unsigned short * pb = mono.ptr<unsigned short>(y);
			if (rgb.channels() == 3)
				for (int x = 0, xx = 0; xx < rgb.cols; xx++, x += 3) {
				pb[xx] = pa[x] + pa[x + 1] + pa[x + 2];
				}
			else
				for (int x = 0, xx = -1, i = rgb.channels(); x < rgb.cols*rgb.channels(); x++) {
				if (i == rgb.channels()) {
					xx++;
					pb[xx] = 0;
					i = 0;
				}
				pb[xx] += pa[x];
				i++;
				}
		}
		break;
	case CV_16U:
		mono.create(rgb.rows, rgb.cols, CV_16U);
		for (int y = 0; y < rgb.rows; y++) {
			const unsigned short * pa = rgb.ptr<unsigned short>(y);
			unsigned short * pb = mono.ptr<unsigned short>(y);
			if (rgb.channels() == 3)
				for (int x = 0, xx = 0; xx < rgb.cols; xx++, x += 3) {
				pb[xx] = pa[x] + pa[x + 1] + pa[x + 2];
				}
			else
				for (int x = 0, xx = -1, i = rgb.channels(); x < rgb.cols*rgb.channels(); x++) {
				if (i == rgb.channels()) {
					xx++;
					pb[xx] = 0;
					i = 0;
				}
				pb[xx] += pa[x];
				i++;
				}
		}
		break;
	case CV_16S:
		mono.create(rgb.rows, rgb.cols, CV_16S);
		for (int y = 0; y < rgb.rows; y++) {
			const short * pa = rgb.ptr<short>(y);
			short * pb = mono.ptr<short>(y);
			if (rgb.channels() == 3)
				for (int x = 0, xx = 0; xx < rgb.cols; xx++, x += 3) {
				pb[xx] = pa[x] + pa[x + 1] + pa[x + 2];
				}
			else
				for (int x = 0, xx = -1, i = rgb.channels(); x < rgb.cols*rgb.channels(); x++) {
				if (i == rgb.channels()) {
					xx++;
					pb[xx] = 0;
					i = 0;
				}
				pb[xx] += pa[x];
				i++;
				}
		}
		break;
	case CV_32F:
		mono.create(rgb.rows, rgb.cols, CV_32F);
		for (int y = 0; y < rgb.rows; y++) {
			const float * pa = rgb.ptr<float>(y);
			float * pb = mono.ptr<float>(y);
			for (int x = 0, xx = -1, i = rgb.channels(); x < rgb.cols*rgb.channels(); x++) {
				if (i == rgb.channels()) {
					xx++;
					pb[xx] = 0;
					i = 0;
				}
				pb[xx] += pa[x];
				i++;
			}
		}
		break;
	default:
		CV_Assert(0);
	}
}

/*------------------------------------------Matting CPU-----------------------------------*/

MattingCPU::MattingCPU()
{
	frame_num = -1;
}

void MattingCPU::reset(int out_width_, int out_height_)
{
	frame_num = -1;
	out_width = out_width_;
	out_height = out_height_;
	out_update = 0;
	lost = 0;
	no_update = 0;
	const_updated = 0;
}

bool MattingCPU::tune_parameter(TuningPara & para)
{
	const_update_lock.lock();
	const_update = para;
	const_updated = 1;
	const_update_lock.unlock();
	return true;
}

int MattingCPU::get_lost()
{
	return lost;
}

int MattingCPU::get_no_update()
{
	return no_update;
}

int MattingCPU::get_out(Color32 * tex)
{
	Mat out_mask_buf, out_rgb_buf;
	if (out_update == 0) {
		no_update++;
		return 0;
	}
	CV_Assert(out_mask.type() == CV_32FC1);
	out_lock.lock();
	out_mask_buf = out_mask;
	out_rgb_buf = out_rgb;
	out_mask.release();
	out_rgb.release();
	out_update = 0;
	out_lock.unlock();
	int height = std::min(out_mask_buf.rows, out_height);
	int width = std::min(out_mask_buf.cols, out_width);
	for (int y = 0; y < height; y++) {
		float * p_mask = out_mask_buf.ptr<float>(y);
		unsigned char * p_out_rgb = out_rgb_buf.ptr<unsigned char>(y);
		Color32 * p_tex = &tex[(height - y - 1) *width];
		for (int x = 0; x < width; x++) {
			p_tex[x].a = (unsigned char)(p_mask[x] * 255);
			p_tex[x].b = p_out_rgb[XB(x)];
			p_tex[x].g = p_out_rgb[XG(x)];
			p_tex[x].r = p_out_rgb[XR(x)];
		}
	}
	LOG(DBG_LEVEL, "MattingCPU get output");
	return 1;
}

void MattingCPU::process(Mat & src)
{
	Mat frame;
	do_preprocess(src, frame);
	CV_Assert(frame.type() == CV_8UC3);
	GammaCorrection(frame, Const.GAMMA);
	//加入下采样，修改一些参数；
	int sz_w, sz_h;
	sz_w = frame.cols*scale;
	sz_h = frame.rows*scale;
	Mat frame_bak, frame_resized;
	resize(frame, frame_resized, Size(sz_w, sz_h));
	frame_bak = frame;
	frame = frame_resized;
#ifdef DEBUG_DZH
	//cvNamedWindow("source", CV_WINDOW_FREERATIO);
	//imshow("source", frame);
	Mat result;
	result.create(frame.rows, frame.cols, CV_8UC3);
	cvWaitKey(10);
#endif

	Mat motion_diff_rgb, motion_diff_rgb_filted, frame_yuv, bg_diff_yuv, bg_diff_filted, alpha_filter_F, alpha_filter, fg_maybe, fg_sure_d;
	
	frame_num++;
	if (const_updated) {
		const_update_lock.lock();
		const_updated = 0;
		transform_para(Const, const_update);
		const_update_lock.unlock();
	}
	
	if (frame_num == 0) {
		frame.convertTo(frame_rgb_pre, CV_32FC3, 1.0/255);
		static_num.create(frame.rows, frame.cols, CV_8U);
		static_num.setTo(0);
		is_bg.create(frame.rows, frame.cols, CV_8U);
		is_bg.setTo(0);
		is_body.create(frame.rows, frame.cols, CV_8U);
		is_body.setTo(0);
		fg_sure.create(frame.rows, frame.cols, CV_8U);
		fg_sure.setTo(0);
		bg_yuv.create(frame.rows, frame.cols, CV_32FC3);
		bg_yuv.setTo(Scalar(0, 1.0, 0));
		return;
	}

	if (frame_num > 2) {		
		out_lock.lock();
		if (out_update == 1)
			lost++;		
		out_rgb = out_rgb_buf; 
		if (Const.show_content == 0)
			out_mask = mask;
		else
		if (Const.show_content == 1) {
			out_mask.create(mask.rows, mask.cols, CV_32F);
			out_mask.setTo(1.0);
		}
		else
		if (Const.show_content == 2) {
			Mat bg;
			bg_yuv.convertTo(bg, CV_8UC3, 255.0);
			cvtColor(bg, bg, CV_YCrCb2BGR);
			cv::resize(bg, out_rgb, Size(out_width, out_height));
			out_mask.create(mask.rows, mask.cols, CV_32F);
			out_mask.setTo(1.0);
		}
		out_update = 1;
		out_lock.unlock();
		out_rgb_buf.release();
		mask.release();
	}
	CV_Assert(frame.size() == frame_rgb_pre.size());
	frame.convertTo(frame_rgb, CV_32FC3, 1.0 / 255);
	subtract(frame_rgb, frame_rgb_pre, motion_diff_rgb);
	cv::boxFilter(motion_diff_rgb, motion_diff_rgb_filted, -1, Size(5, 5), Point(-1, -1), true);//warning

	cvtColor(frame_rgb, frame_yuv, CV_BGR2YCrCb);
	subtract(frame_yuv, bg_yuv, bg_diff_yuv);
	//噪声统计
	double noise_avg = 0;
	int noise_count = 0;
	static Mat frame_yuv_pre;
	if (frame_yuv_pre.empty())
	{
		frame_yuv_pre.create(frame_yuv.rows, frame_yuv.cols, CV_32F);
	}
	for (int y = 1; y < frame_rgb.rows - 1; y++) 
	{
		const float * py_dec = frame_yuv.ptr<float>(y - 1);
		const float * py_inc = frame_yuv.ptr<float>(y + 1);
		const float * p_frame_yuv = frame_yuv.ptr<float>(y);
		float * p_frame_yuv_pre = frame_yuv_pre.ptr<float>(y);
		const float * p_motion_diff_rgb_filted = motion_diff_rgb_filted.ptr<float>(y);
		unsigned char * p_static_num = static_num.ptr<unsigned char>(y);
		const unsigned char * p_is_bg = is_bg.ptr<unsigned char>(y);
		const unsigned char * p_is_body = is_body.ptr<unsigned char>(y);
		float* p_bg_yuv = bg_yuv.ptr<float>(y);		
		const float * p_bg_diff_yuv = bg_diff_yuv.ptr<float>(y);

		for (int x = 1; x < frame_rgb.cols - 1; x++) 
		{
			int x3 = x * 3;
			float edge_offset = max(fabs(py_dec[x3 - 3] - py_inc[x3 + 3]),
				fabs(py_dec[x3 + 3] - py_inc[x3 - 3])) / 2;
			
			float motion_diff = fabs(p_motion_diff_rgb_filted[XB(x)]) + fabs(p_motion_diff_rgb_filted[XG(x)]) + fabs(p_motion_diff_rgb_filted[XR(x)]);
			if (motion_diff < edge_offset + Const.motion_TH_f) 
			{ 
				//dzh modify 
				p_static_num[x] = min(p_static_num[x] + 1, (int)Const.static_MAX);
			} else
				p_static_num[x] = 0;				
			if (p_bg_yuv[x3] == 0 && p_bg_yuv[x3 + 1] == 1.0 && p_bg_yuv[x3 + 2] == 0) 
			{
				if (p_static_num[x] >= Const.init_static_num) {
					p_bg_yuv[XB(x)] = p_frame_yuv[XB(x)];
					p_bg_yuv[XG(x)] = p_frame_yuv[XG(x)];
					p_bg_yuv[XR(x)] = p_frame_yuv[XR(x)];
				}
			}
			else {
				float update_speed;
				if (p_is_bg[x] == 255 && p_static_num[x] >= Const.static_NUM)
				{
					update_speed = Const.static_SPEED_f;
					noise_avg += fabs(p_frame_yuv_pre[XB(x)] - p_frame_yuv[XB(x)]);
					noise_count++;
				}			
				else if (p_is_body[x] == 255 && p_static_num[x] >= Const.long_static_NUM)//目前不起作用
					update_speed = Const.long_SPEED_f;
				else
					update_speed = 0;
								
				p_bg_yuv[XB(x)] = (p_bg_diff_yuv[XB(x)] > 0) ? (p_bg_yuv[XB(x)] + update_speed) : (p_bg_yuv[XB(x)] - update_speed);
				p_bg_yuv[XG(x)] = (p_bg_diff_yuv[XG(x)] > 0) ? (p_bg_yuv[XG(x)] + update_speed) : (p_bg_yuv[XG(x)] - update_speed);
				p_bg_yuv[XR(x)] = (p_bg_diff_yuv[XR(x)] > 0) ? (p_bg_yuv[XR(x)] + update_speed) : (p_bg_yuv[XR(x)] - update_speed);
			}
		}
	}
	frame_yuv_pre = frame_yuv;
	noise_avg = 255*noise_avg / noise_count;
	cout << (float)noise_avg << endl;
#ifdef DEBUG_DZH
	Mat bg_rgb_tmp;
	bg_yuv.convertTo(bg_rgb_tmp, CV_8UC3, 255.0);
	cvtColor(bg_rgb_tmp, bg_rgb_tmp, CV_YCrCb2BGR);
	//cvNamedWindow("bkground", CV_WINDOW_FREERATIO);
	//imshow("bkground", bg_rgb_tmp);
#if WRITETODISK==1
	char buff_bg[64];
	sprintf_s(buff_bg, "%s/bg/bg%d.bmp", img_name.c_str(), frame_num);
	//imwrite(buff_bg, bg_rgb_tmp);
#endif
#endif

	cv::boxFilter(bg_diff_yuv, bg_diff_filted, -1, Size(5, 5), Point(-1, -1), true);//warning
	fg_maybe.create(frame.rows, frame.cols, CV_8U);
	fg_maybe.setTo(0);
	if (frame_num > 20) {
		fg_sure.setTo(0);

		int _alpha_top = alpha_top*scale;
		int _alpha_bottom = alpha_bottom*scale;
		int _alpha_left = alpha_left*scale;
		int _alpha_right = alpha_right*scale;
				
		for (int y = _alpha_top; y < frame_rgb.rows - _alpha_bottom; y++)
		{
			const float* p_bg_diff_filted = bg_diff_filted.ptr<float>(y);
			unsigned char* p_fg_sure = fg_sure.ptr<unsigned char>(y);
			unsigned char* p_fg_maybe = fg_maybe.ptr<unsigned char>(y);			
			unsigned char* p_is_body = is_body.ptr<unsigned char>(y);
			for (int x = _alpha_left; x < frame_rgb.cols - _alpha_right; x++) 
			{
				float bg_diff_abs_y = fabs(p_bg_diff_filted[XB(x)]);
				float bg_diff_abs_u = fabs(p_bg_diff_filted[XG(x)]);
				float bg_diff_abs_v = fabs(p_bg_diff_filted[XR(x)]);

				bg_diff_abs_y = max(0.0f, bg_diff_abs_y - Const.luma_offset_f);
				bg_diff_abs_u = bg_diff_abs_u * Const.u_gain_f;
				bg_diff_abs_v = bg_diff_abs_v * Const.v_gain_f;

				float bg_diff_all = (bg_diff_abs_y + bg_diff_abs_u + bg_diff_abs_v)*(p_fg_sure[x] + 1);
				float motion_th = Const.alpha_TH_f;				
				if ((y >= body_top - 1) && (y <= body_bottom - 1) && (x >= body_left - 1) && (x <= body_right - 1))
				{
					p_is_body[x] = 255;
					motion_th = Const.alpha_TH_f / 2;
				}
				else
				{
					p_is_body[x] = 0;
				}	
				if (bg_diff_all > motion_th * 2) {
					p_fg_maybe[x] = 255;
					p_fg_sure[x] = 255;									
				}
				else 
				{				
					if (bg_diff_all > motion_th)
					{
						p_fg_maybe[x] = 255;
						p_fg_sure[x] = 0;
					}
					else 
					{
						p_fg_maybe[x] = 0;
						p_fg_sure[x] = 0;
					}
				}				
			}
		}
	}

	Mat element = getStructuringElement(MORPH_RECT, Size(21, 21), Point(10, 10));
	dilate(fg_sure, fg_sure_d, element);
	//is_bg = (fg_sure_d == 0);

	Mat alpha_raw;
	alpha_raw = (fg_maybe & fg_sure_d);

	CV_Assert(alpha_raw.type() == CV_8U);
	char buff_contour_before[64];
	sprintf_s(buff_contour_before, "%s/contour_before/contour%d.bmp", img_name.c_str(), frame_num);
	Mat contour_before = alpha_raw.clone();
	//imwrite(buff_contour_before, alpha_raw);
	FindContour(alpha_raw, Const.block_th*scale*scale);
	char buff_contour_after[64];
	sprintf_s(buff_contour_after, "%s/contour_after/contour%d.bmp", img_name.c_str(), frame_num);
	Mat contour_after = alpha_raw.clone();
	//imwrite(buff_contour_after, alpha_raw);
	FindGeometry(alpha_raw, frame_bak, Const.rasie_hands_radius, (float)out_width / frame.cols, (float)out_height / frame.rows);
	is_bg_raw = (alpha_raw == 0);

	Mat element2 = getStructuringElement(MORPH_RECT, Size(11, 11), Point(1, 1));
	cv::erode(is_bg_raw, is_bg, element2);
#ifdef DEBUG_DZH
	//cvNamedWindow("alpha", CV_WINDOW_FREERATIO);
	//imshow("alpha", is_bg);
	//cvWaitKey(10);
#if WRITETODISK==1
	char buff_alpha[64];
	sprintf_s(buff_alpha, "%s/mask/alpha%d.bmp", img_name.c_str(), frame_num);
	//imwrite(buff_alpha, is_bg);
#endif
#endif	
	Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
	cv::erode(alpha_raw, alpha_erode, element1);
	//CV_Assert(alpha_erode.type() == CV_8U);
	//FindContour(alpha_erode, Const.block_th);
	//is_bg = (alpha_erode == 0);
	/*char buf_msk[64];
	sprintf_s(buf_msk, "mask/msk%d.png", frame_num);
	imwrite(buf_msk, alpha_erode);*/
	

	alpha_erode.convertTo(alpha_filter_F, CV_32F, 1.0 / 255);
	//cv::boxFilter(alpha_filter_F, alpha_filter, -1, Size(Const.alpha_filter_r, Const.alpha_filter_r), Point(-1, -1), true);
	int alpha_filter_r = Const.alpha_filter_r*scale;
	if (alpha_filter_r % 2 == 0)alpha_filter_r++;
	cv::boxFilter(alpha_filter_F, alpha_filter, -1, Size(alpha_filter_r, alpha_filter_r), Point(-1, -1), true);
	alpha_filter.convertTo(alpha_filter, CV_32F, 2.0, -1.0);
	cv::max(alpha_filter, 0, alpha_filter);

	frame_rgb_pre = frame_rgb;
	frame_rgb.release();
	cv::resize(frame, out_rgb_buf, Size(out_width, out_height));
	cv::resize(alpha_filter, mask, Size(out_width, out_height));

#ifdef DEBUG_DZH
	//摄像机调试显示结果
	if (frame_num > 20)
	{
		for (int y = 0; y < frame.rows; y++) {
			const unsigned char * p_frame = frame.ptr<unsigned char>(y);
			const float * p_mask = alpha_filter.ptr<float>(y);
			unsigned char * p_result = result.ptr<unsigned char>(y);
			for (int x = 0; x < frame.cols; x++)
			{
				//cout << frame.rows << "," << result.rows << "," << alpha_filter.rows << endl;
				p_result[XB(x)] = p_frame[XB(x)] * p_mask[x];
				p_result[XG(x)] = p_frame[XG(x)] * p_mask[x];
				p_result[XR(x)] = p_frame[XR(x)] * p_mask[x] + 255 * (1 - p_mask[x]);
			}
		}
		//cvNamedWindow("result", CV_WINDOW_FREERATIO);
		//imshow("result", result);
		//cvWaitKey(10);
#if WRITETODISK==1
		char buff_rst[64];
		sprintf_s(buff_rst, "%s/result/rst%d.jpg", img_name.c_str(), frame_num);
		//imwrite(buff_rst, result);
	
		Mat dst(frame.rows, frame.cols, frame.type());
		int x_half = (dst.cols + 1) / 2;
		int y_half = (dst.rows + 1) / 2;
		for (int y = 0; y < y_half; y++)
		{
			const unsigned char * p_frame = frame.ptr<unsigned char>(y * 2);
			const unsigned char * p_result = result.ptr<unsigned char>(y * 2);
			unsigned char* p_dst = dst.ptr<unsigned char>(y);
			for (int x = 0; x < dst.cols; x++)
			{
				if (x < x_half)
				{
					p_dst[x * 3] = p_frame[x * 6];
					p_dst[x * 3 + 1] = p_frame[x * 6 + 1];
					p_dst[x * 3 + 2] = p_frame[x * 6 + 2];
				}
				else
				{
					p_dst[x * 3] = p_result[(x - x_half) * 6];
					p_dst[x * 3 + 1] = p_result[(x - x_half) * 6 + 1];
					p_dst[x * 3 + 2] = p_result[(x - x_half) * 6 + 2];
				}
			}
		}
		for (int y = y_half; y < dst.rows; y++)
		{
			const unsigned char * p_contour_before = contour_before.ptr<unsigned char>((y - y_half) * 2);
			const unsigned char * p_contour_after = contour_after.ptr<unsigned char>((y - y_half) * 2);
			unsigned char* p_dst = dst.ptr<unsigned char>(y);
			for (int x = 0; x < dst.cols; x++)
			{
				if (x < x_half)
				{
					p_dst[x * 3] = p_contour_before[x * 2];
					p_dst[x * 3 + 1] = p_contour_before[x * 2];
					p_dst[x * 3 + 2] = p_contour_before[x * 2];
				}
				else
				{
					p_dst[x * 3] = p_contour_after[(x - x_half) * 2];
					p_dst[x * 3 + 1] = p_contour_after[(x - x_half) * 2];
					p_dst[x * 3 + 2] = p_contour_after[(x - x_half) * 2];
				}
			}
		}


		imwrite(buff_rst, dst);
#endif
	}
	static int frame_count = -1;
	frame_count++;
	if (frame_num > 1)
	{
		double now_time = (double)getTickCount();
		cout << "frames:" << frame_num;
		cout << "   process: " << (now_time - preve) / getTickFrequency() << " ms";
		//cout << "   rate_avg:" << frame_count*getTickFrequency() / (now_time - start2);
		cout << "   delay:" << 120 + (now_time - camera_recv) / 1000 << endl;
		preve = now_time;
	}
	else
	{
		start2 = (double)getTickCount();
		preve = (double)getTickCount();
	}
	/*static int count = 1;
	count++;
	std::cout << "camera gives:" << count << std::endl;*/
#endif // DEBUG_DZH
}
#if HAVE_GPU==1
using namespace cv::gpu;
class GpuHeap
{
public:
	GpuHeap() {
		datastart = dataend = data = NULL;
	}
	GpuHeap(size_t size, unsigned align1 = 0x200, unsigned align2 = 0x1000) {
		datastart = dataend = data = NULL;
		reset(size, align1, align2);
	}
	~GpuHeap() {
#ifndef _USRDLL
		if (datastart!=NULL)
			checkCudaErrors(cudaFree(datastart));
#endif
		datastart = dataend = data = NULL;
	}

	void reset(size_t size, unsigned align1 = 0x200, unsigned align2 = 0x1000) {
		if (datastart != NULL)
			checkCudaErrors(cudaFree(datastart));
		checkCudaErrors(cudaMalloc((void**)&data, size));
		CV_Assert(align1 <= align2);
		datastart = data;
		dataend = data + size;
		a1 = align1;
		a2 = align2;
	}
	void alloc_pitch(int rows, int cols, int type, gpu::GpuMat &m) {
		size_t step;
		size_t min_step = cols * CV_ELEM_SIZE(type);
		unsigned long long d, e;
		step = (min_step & ~(a1 - 1)) + a1;
		m = gpu::GpuMat(rows, cols, type, data, step);
		data += step * rows;
		d = (unsigned long long)data;
		e = (unsigned long long)dataend;
		CV_Assert(d < e);
		d = (d & ~(a2 - 1)) + a2;
		data = (uchar *)d;
	}
	void free(gpu::GpuMat & m) {
		unsigned long long d, f;
		d = (unsigned long long)data;
		f = (unsigned long long)m.datastart;
		CV_Assert(d >f);
		data = m.datastart;
		m.release();
	}
	void freeall() {
		data = datastart;
	}
protected:
	uchar * data;
	uchar * datastart;
	uchar * dataend;
	unsigned long long a1, a2;
};
void trace_bg(PtrStepSzb motion_diff_rgb_filted0, PtrStepSzb motion_diff_rgb_filted1, PtrStepSzb motion_diff_rgb_filted2,
	PtrStepSzb frame_yuv, PtrStepSzb bg_yuv, PtrStepSzb bg_diff_yuv, PtrStepSzb static_num, PtrStepSzb is_bg, PtrStepSzb is_body, cudaStream_t stream);

void update_mask_bg(PtrStepSzb bg_diff_filted0, PtrStepSzb bg_diff_filted1, PtrStepSzb bg_diff_filted2, 
	PtrStepSzb fg_sure, PtrStepSzb fg_maybe, PtrStepSzb is_body, cudaStream_t stream);

void tune_gpu_parameter(TuningParaFloat * c);
void update_host_para(HostPara * p);
static GpuHeap gpu_heap;

static void convertRGB2RGBA(const Mat & img_rgb, const Mat & alpha, Mat & img_out)
{
	CV_Assert(img_out.cols == img_rgb.cols && img_out.cols == alpha.cols && img_out.rows == img_rgb.rows && img_out.rows == alpha.rows);
	CV_Assert(img_rgb.type() == CV_8UC3 && alpha.type() == CV_8U && img_out.type() == CV_8UC4);
	Mat in[] = { img_rgb, alpha };
	int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
	mixChannels(in, 2, &img_out, 1, from_to, 4);
}	

MattingGPU::MattingGPU() 
{
	frame_num = -1;	
	checkCudaErrors(cudaSetDevice(0));
	face_detector.load("haarcascades/haarcascade_frontalface_alt2.xml");
}

void MattingGPU::reset(int out_width_, int out_height_)
{
	frame_num = -1;
	out_width = out_width_;
	out_height = out_height_;
	out_update = 0;
	lost = 0;
	no_update = 0;
	const_updated = 0;
}

bool MattingGPU::tune_parameter(TuningPara & para)
{
	const_update_lock.lock();
	const_update = para;
	const_updated = 1;
	const_update_lock.unlock();
	return true;
}

#ifdef DEBUG_DZH
Mat result, pre_frame;
#endif
void MattingGPU::process(Mat & src)
{
	Mat frame;
	do_preprocess(src, frame);
	CV_Assert(frame.type() == CV_8UC3);
	GammaCorrection(frame, Const.GAMMA);
	//加入下采样，修改一些参数；
	int sz_w, sz_h;
	sz_w = frame.cols*scale;
	sz_h = frame.rows*scale;
	Mat frame_bak, frame_resized;
	resize(frame, frame_resized, Size(sz_w, sz_h));
	frame_bak = frame;
	frame = frame_resized;

#ifdef DEBUG_DZH
	cvNamedWindow("source", CV_WINDOW_FREERATIO);
	imshow("source", frame);
	result.create(frame.rows, frame.cols, CV_8UC3);
	cvWaitKey(10);
#endif

	GpuMat motion_diff_rgb, split_buf[CHANNEL], motion_diff_rgb_filted[CHANNEL], frame_yuv, bg_diff_yuv, bg_diff_filted[CHANNEL];
	GpuMat fg_maybe, fg_sure, fg_sure_d, fg_sure_dilate, alpha_raw;
	Mat alpha_filter_F, alpha_filter;

	if (const_updated) {
		const_update_lock.lock();
		const_updated = 0;
		transform_para(Const, const_update);
		const_update_lock.unlock();
		tune_gpu_parameter(&Const);
	}
	frame_num++;
	if (frame_num == 0) {
#if PROFILE ==1
		cpu_process = cpu_wait = gpu_launch = 0;
#endif
		gpu_heap.reset(0x8000000);
		frame_rgb_cpu.create(frame.rows, frame.cols, CV_8UC3);
		frame.copyTo(frame_rgb_cpu.createMatHeader());
		frame_rgb_raw_gpu.create(frame.rows, frame.cols, CV_8UC3);
		frame_rgb_raw_gpu.upload(frame_rgb_cpu);
		frame_rgb_gpu.create(frame.rows, frame.cols, CV_32FC3);
		frame_rgb_pre_gpu.create(frame.rows, frame.cols, CV_32FC3);
		frame_rgb_raw_gpu.convertTo(frame_rgb_pre_gpu, CV_32FC3, 1.0 / 255);
		bg_yuv_gpu.create(frame.rows, frame.cols, CV_32FC3);
		bg_yuv_gpu.setTo(Scalar(0, 1.0f, 0, 0));
		bg_yuv_cpu.create(frame.rows, frame.cols, CV_32FC3);		
		static_num_gpu.create(frame.rows, frame.cols, CV_8U);
		static_num_gpu.setTo(0);
		is_bg_gpu.create(frame.rows, frame.cols, CV_8U);
		is_bg_gpu.setTo(255);
		is_bg_cpu.create(frame.rows, frame.cols, CV_8U);		
		is_body_gpu.create(frame.rows, frame.cols, CV_8U);
		is_body_gpu.setTo(0);
		alpha_erode_gpu.create(frame.rows, frame.cols, CV_8U);
		alpha_erode_gpu.setTo(0);
		alpha_erode_cpu.create(frame.rows, frame.cols, CV_8U);

		Mat kx = Mat::ones(5, 1, CV_32F);
		kx = kx / 5.0f;
		box_filter = gpu::createSeparableLinearFilter_GPU(CV_32FC1, CV_32FC1, kx, kx, box_buf);
		/*Mat element = getStructuringElement(MORPH_RECT, Size(21, 21), Point(10, 10));
		dilate_filter = gpu::createMorphologyFilter_GPU(MORPH_DILATE, CV_8U, element, dilate_buf);*/
		Mat kd = Mat::ones(15, 1, CV_16U);
		dilate_filter = gpu::createSeparableLinearFilter_GPU(CV_8U, CV_16U, kd, kd, dilate_buf);
		Mat element1 = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		erode_filter = gpu::createMorphologyFilter_GPU(MORPH_ERODE, CV_8U, element1, erode_buf);
		return;
	}
	CV_Assert(frame.size() == frame_rgb_cpu.size());
	frame.copyTo(frame_rgb_cpu.createMatHeader());
#if PROFILE ==1
	double start = (double)getTickCount();
#endif
	if (frame_num > 1) {
		stream.waitForCompletion();
		HostPara host_para;
		host_para.body_top = body_top;
		host_para.body_bottom = body_bottom;
		host_para.body_left = body_left;
		host_para.body_right = body_right;
		update_host_para(&host_para);
		frame_rgb_gpu.swap(frame_rgb_pre_gpu);
		if (frame_num > 2)
			stream.enqueueUpload(is_bg_cpu, is_bg_gpu);
	}
#if PROFILE ==1
	double gpu_complete = ((double)getTickCount() - start) / getTickFrequency();
	cpu_wait += gpu_complete;
#endif
	stream.enqueueUpload(frame_rgb_cpu, frame_rgb_raw_gpu);
	stream.enqueueConvert(frame_rgb_raw_gpu, frame_rgb_gpu, CV_32FC3, 1.0 / 255);
	for (int i = 0; i < CHANNEL; i++) {
		gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32F, split_buf[i]);
		gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32F, motion_diff_rgb_filted[i]);
	}

	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32FC3, motion_diff_rgb);
	gpu::subtract(frame_rgb_gpu, frame_rgb_pre_gpu, motion_diff_rgb, cv::gpu::GpuMat(), -1, stream);
	gpu::split(motion_diff_rgb, split_buf, stream);

	for (int i = 0; i < CHANNEL; i++)
		box_filter->apply(split_buf[i], motion_diff_rgb_filted[i], Rect(0, 0, -1, -1), stream);

	gpu_heap.free(motion_diff_rgb);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_8UC1, fg_maybe);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_8UC1, fg_sure);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32FC3, bg_diff_yuv);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32FC3, frame_yuv);
	gpu::cvtColor(frame_rgb_gpu, frame_yuv, CV_BGR2YCrCb, 0, stream);
	gpu::subtract(frame_yuv, bg_yuv_gpu, bg_diff_yuv, cv::gpu::GpuMat(), -1, stream);
	trace_bg(motion_diff_rgb_filted[0], motion_diff_rgb_filted[1], motion_diff_rgb_filted[2], 
		frame_yuv, bg_yuv_gpu, bg_diff_yuv, static_num_gpu, is_bg_gpu, is_body_gpu, StreamAccessor::getStream(stream));
	gpu_heap.free(frame_yuv);
	for (int i=0; i<CHANNEL; i++)
		gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_32F, bg_diff_filted[i]);
	gpu::split(bg_diff_yuv, split_buf, stream);
	for (int i = 0; i < CHANNEL; i++)
		box_filter->apply(split_buf[i], bg_diff_filted[i], Rect(0, 0, -1, -1), stream);
	stream.enqueueMemSet(fg_maybe, Scalar::all(0));
	stream.enqueueMemSet(fg_sure, Scalar::all(0));

	if (frame_num > 20) {		
		update_mask_bg(bg_diff_filted[0], bg_diff_filted[1], bg_diff_filted[2], 
			fg_sure, fg_maybe, is_body_gpu, StreamAccessor::getStream(stream));
	}

	for (int i = CHANNEL - 1; i >= 0; i--)
		gpu_heap.free(bg_diff_filted[i]);
	gpu_heap.free(bg_diff_yuv);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_8UC1, fg_sure_d);

	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_16UC1, fg_sure_dilate);
	dilate_filter->apply(fg_sure, fg_sure_dilate, Rect(0, 0, -1, -1), stream);
	gpu::compare(fg_sure_dilate, 0, fg_sure_d, CMP_GT, stream);

	//gpu::compare(fg_sure_d, 0, is_bg_gpu, CMP_EQ, stream);
	gpu_heap.alloc_pitch(frame.rows, frame.cols, CV_8UC1, alpha_raw);
	gpu::bitwise_and(fg_maybe, fg_sure_d, alpha_raw, cv::gpu::GpuMat(), stream);

	//erode_filter->apply(alpha_raw, alpha_erode_gpu, Rect(0, 0, -1, -1), stream); 

	gpu::multiply(alpha_raw, Scalar::all(255.0), alpha_erode_gpu, 1.0, -1, stream);

#if PROFILE == 1
	double gpu_launch_complete = ((double)getTickCount() - start) / getTickFrequency() - gpu_complete;
	gpu_launch += gpu_launch_complete;
#endif	
	if (frame_num > 1) {
		Mat alpha_raw;
		alpha_erode_cpu.createMatHeader().copyTo(alpha_raw);
		
		FindContour(alpha_raw, Const.block_th*scale*scale);
		FindGeometry(alpha_raw, out_rgb_buf, Const.rasie_hands_radius, (float)out_width / frame.cols, (float)out_height / frame.rows);
#ifdef DEBUG_DZH
		cvNamedWindow("alpha", CV_WINDOW_FREERATIO);
		imshow("alpha", alpha_raw);
		cvWaitKey(10);
#endif
		Mat temp = (alpha_raw == 0);
		Mat element_is_bg_erode = getStructuringElement(MORPH_RECT, Size(11, 11), Point(1, 1));
		erode(temp, temp, element_is_bg_erode);
		temp.copyTo(is_bg_cpu.createMatHeader());
		Mat element_alpha_erode = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
		Mat alpha_erode;
		cv::erode(alpha_raw, alpha_erode, element_alpha_erode);

		alpha_erode.convertTo(alpha_filter_F, CV_32F, 1.0 / 255);
		//cv::boxFilter(alpha_filter_F, alpha_filter, -1, Size(Const.alpha_filter_r, Const.alpha_filter_r), Point(-1, -1), true);
		int alpha_filter_r = Const.alpha_filter_r*scale;
		if (alpha_filter_r % 2 == 0)alpha_filter_r++;
		cv::boxFilter(alpha_filter_F, alpha_filter, -1, Size(alpha_filter_r, alpha_filter_r), Point(-1, -1), true);
		alpha_filter.convertTo(alpha_filter, CV_32F, 2.0, -1.0);
		cv::max(alpha_filter, 0, alpha_filter);

#ifdef DEBUG_DZH
		Mat bg;
		bg_yuv_cpu.createMatHeader().convertTo(bg, CV_8UC3, 255.0);
		cvtColor(bg, bg, CV_YCrCb2BGR);
		stream.enqueueDownload(bg_yuv_gpu, bg_yuv_cpu);
		cvNamedWindow("bkground", CV_WINDOW_FREERATIO);
		imshow("bkground", bg);
		cvWaitKey(10);
#endif
		Mat out_mask_buf;
		resize(alpha_filter, out_mask_buf, Size(out_width, out_height));
		out_lock.lock();
		if (out_update == 1)
			lost++;
		out_rgb = out_rgb_buf.clone();
		if (Const.show_content == 0)
			out_mask = out_mask_buf.clone();
		else
		if (Const.show_content == 1) {
			out_mask.create(out_mask_buf.rows, out_mask_buf.cols, CV_32F);
			out_mask.setTo(1.0);
		}
		else
		if (Const.show_content == 2) {
			Mat bg;
			bg_yuv_cpu.createMatHeader().convertTo(bg, CV_8UC3, 255.0);
			cvtColor(bg, bg, CV_YCrCb2BGR);
			cv::resize(bg, out_rgb, Size(out_width, out_height));
			out_mask.create(out_mask_buf.rows, out_mask_buf.cols, CV_32F);
			out_mask.setTo(1.0);
			stream.enqueueDownload(bg_yuv_gpu, bg_yuv_cpu);
		}
		out_update = 1;
		out_lock.unlock();
		out_rgb_buf.release();
	}

	stream.enqueueDownload(alpha_erode_gpu, alpha_erode_cpu);
	gpu_heap.freeall();
	resize(frame_bak, out_rgb_buf, Size(out_width, out_height));
#ifdef DEBUG_DZH
	//摄像机调试显示结果
	if (frame_num > 20&&frame.cols==pre_frame.cols)
	{
		for (int y = 0; y < frame.rows; y++) {
			const unsigned char * p_frame = pre_frame.ptr<unsigned char>(y);
			const float * p_mask = alpha_filter.ptr<float>(y);
			unsigned char * p_result = result.ptr<unsigned char>(y);
			for (int x = 0; x < frame.cols; x++)
			{
				//cout << frame.rows << "," << result.rows << "," << alpha_filter.rows << endl;
				p_result[XB(x)] = p_frame[XB(x)] * p_mask[x];
				p_result[XG(x)] = p_frame[XG(x)] * p_mask[x];
				p_result[XR(x)] = p_frame[XR(x)] * p_mask[x] + 255 * (1 - p_mask[x]);
			}
		}
		cvNamedWindow("result", CV_WINDOW_FREERATIO);
		imshow("result", result);
		cvWaitKey(10);
	}
	pre_frame = frame;
	static int frame_count = -1;
	frame_count++;
	if (frame_num > 1)
	{
		double now_time = (double)getTickCount();
		cout << "frames:" << frame_num;
		cout << "   rate:" << getTickFrequency() / (now_time - preve);
		//cout << "   rate_avg:" << frame_count*getTickFrequency() / (now_time - start2);
		cout << "   delay:" << 120 + (now_time - camera_recv) / 1000 << endl;
		preve = now_time;
	}
	else
	{
		start2 = (double)getTickCount();
		preve = (double)getTickCount();
	}
	/*static int count = 1;
	count++;
	std::cout << "camera gives:" << count << std::endl;*/
#endif // DEBUG_DZH
#if PROFILE==1
	double cpu_complete = ((double)getTickCount() - start) / getTickFrequency() - gpu_launch_complete - gpu_complete;
	LOG(DBG_LEVEL, "gpu_wait %f, gpu_launch %f, cpu_process %f", gpu_complete, gpu_launch_complete, cpu_complete);
	cpu_process += cpu_complete;
#endif
}
//int MattingGPU::get_out(Color32 * tex)
//{
//	if (out_update == 0) 
//	{
//		no_update++;
//		return 0;
//	}
//	Mat dst;
//	dst.data = (uchar*)tex;
//	Mat alpha;
//	out_mask.convertTo(alpha, CV_8U, 255);
//	Mat in[] = { alpha, out_rgb };
//	//int from_to[] = { 0, 0, 1, 3, 2, 2, 3, 1 };
//	int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
//	mixChannels(in, 2, &dst, 1, from_to, 4);
//	imwrite("argb.png", dst);
//	return;
//
//}

int MattingGPU::get_out(Color32 * tex)
{
	Mat out_mask_buf, out_rgb_buf;
	if (out_update == 0) {
		no_update++;
		return 0;
	}
	CV_Assert(out_mask.type() == CV_32FC1);
	out_lock.lock();
	out_mask_buf = out_mask.clone();
	out_rgb_buf = out_rgb.clone();
	//out_mask.release();
	//out_rgb.release();
	out_update = 0;
	out_lock.unlock();
	int height = std::min(out_mask_buf.rows, out_height);
	int width = std::min(out_mask_buf.cols, out_width);
	for (int y = 0; y < height; y++) {
		float * p_mask = out_mask_buf.ptr<float>(y);
		unsigned char * p_out_rgb = out_rgb_buf.ptr<unsigned char>(y);
		Color32 * p_tex = &tex[(height - y - 1) *width];
		for (int x = 0; x < width; x++) {
			p_tex[x].a = (unsigned char)(p_mask[x] * 255);
			p_tex[x].b = p_out_rgb[3 * x];
			p_tex[x].g = p_out_rgb[3 * x + 1];
			p_tex[x].r = p_out_rgb[3 * x + 2];
		}
	}
	LOG(DBG_LEVEL, "MattingGPU get output");
	return 1;
}


int MattingGPU::get_lost()
{
	return lost;
}

int MattingGPU::get_no_update()
{
	return no_update;
}
#endif
void alpha_mixer(const Mat & img_in, const Mat & mask, Mat & img_out)
{
	CV_Assert(img_in.type() == CV_8UC3 && mask.type() == CV_8U);
	img_out.create(img_in.rows, img_in.cols, CV_8UC3);
	for (int y = 0; y < img_in.rows; y++) {
		const unsigned char * p_img_in = img_in.ptr<unsigned char>(y);
		const unsigned char * p_mask = mask.ptr<unsigned char>(y);
		unsigned char * p_img_out = img_out.ptr<unsigned char>(y);
		for (int x = 0; x < img_in.cols; x++)
			if (p_mask[x]) {
				p_img_out[3 * x] = p_img_in[3 * x];
				p_img_out[3 * x+1] = p_img_in[3 * x+1];
				p_img_out[3 * x+2] = p_img_in[3 * x+2];
			}
			else {
				p_img_out[3 * x] = 0;
				p_img_out[3 * x + 1] = 0;
				p_img_out[3 * x + 2] = 0;
			}
	}
}

static unsigned img_idx;
bool get_nextimg(Mat & m)
{	 
	char num[20];
	sprintf_s(num, "%04d.bmp", img_idx);
	string file;
	file = input_path + num;
	m = imread(file);
	if (m.data == NULL) {
		sprintf_s(num, "%04d.tif", img_idx);
		file = input_path + num;
		m = imread(file);	
	}
	img_idx++;
	if (m.data == NULL)
		return false;
	else
		return true;
}

static int produce_rate = 40;
static int rounds;
void process_disk_img_thread()
{
	Mat frame_rgb;
	double start, read_disk_total=0, process_total=0;
	int frame_num = 0;
	do {
		img_idx = frame_start;
		while (1) {
			start = (double)getTickCount();
			frame_num++;
			if (!get_nextimg(frame_rgb))
				break;
			double read_disk = ((double)getTickCount() - start) / getTickFrequency();
			read_disk_total += read_disk;
			LOG(DBG_LEVEL, "disk read %d", frame_num);
			matting->process(frame_rgb);
			double process = ((double)getTickCount() - start) / getTickFrequency() - read_disk;
			process_total += process;
			double t = (read_disk + process) *1000;
			if (t < 1000 / produce_rate) {
				chrono::milliseconds dura(1000 / produce_rate - (int)t);
				this_thread::sleep_for(dura);
			}
		}
		cout << "produce"<< rounds<< ",img_idx="<<img_idx<<'\n';
	} while (--rounds != 0);
#ifndef _USRDLL
	cout << "average read disk time:" << read_disk_total / frame_num << '\n';
	cout << "average process time:" << process_total / frame_num << '\n';
#if HAVE_GPU==1 && PROFILE==1
	MattingGPU * matting_gpu = dynamic_cast<MattingGPU *>(matting);
	if (matting_gpu != NULL) {
		cout << "average wait time" << matting_gpu->cpu_wait / frame_num << '\n';
		cout << "average launch time" << matting_gpu->gpu_launch / frame_num << '\n';
		cout << "average process time" << matting_gpu->cpu_process / frame_num << '\n';
	}
#endif
#endif
}

void alpha_mixer(Color32 * tex, Mat & img_out, int width, int height)
{
	img_out.create(height, width, CV_8UC3);
	for (int y = 0; y < img_out.rows; y++) {
		Color32 * p_tex = &tex[(height-1-y) * width];
		unsigned char * p_img_out = img_out.ptr<unsigned char>(y);
		for (int x = 0; x < img_out.cols; x++) {
			p_img_out[3 * x] = ((unsigned)p_tex[x].b * (unsigned)p_tex[x].a) >>8;
			p_img_out[3 * x + 1] = ((unsigned)p_tex[x].g *(unsigned)p_tex[x].a) >> 8;
			p_img_out[3 * x + 2] = ((unsigned)p_tex[x].r *(unsigned)p_tex[x].a) >> 8;
		}			
	}
}

void write_nextimg(const Mat & m)
{
	static unsigned out_idx = frame_start;
	char num[10];

	sprintf_s(num, "%d.bmp", out_idx++);
	string file;
	file = output_path + num;
	imwrite(file, m);
}

void write_texture(Color32 *tex, int wide, int height)
{
	Mat fg;
	alpha_mixer(tex, fg, wide, height);
	write_nextimg(fg);
}


MattingFifo::MattingFifo()
{	
	buf_update = 0;
	lost = 0;
	frame_num = 0;
}

void MattingFifo::put(Mat & frame)
{
	//Mat tmp_buf = frame.clone();
	LOG(DBG_LEVEL, "frame %d come from camera", frame_num);
	if (buf_update)
		lost++;
	buf_lock.lock();
	mat_buf = frame.clone();
	buf_update = 1;
	frame_get = frame_num;
	buf_lock.unlock();
	frame_num++;
}
#ifdef DEBUG_DZH
void MattingFifo::put2(cv::Mat & frame, double time_recv)
{
	Mat tmp_buf = frame.clone();
	LOG(DBG_LEVEL, "frame %d come from camera", frame_num);
	if (buf_update)
		lost++;
	buf_lock.lock();
	mat_buf = tmp_buf;
	buf_update = 1;
	frame_get = frame_num;
	camera_recv_buf = time_recv;
	buf_lock.unlock();
	frame_num++;
}
bool MattingFifo::get2(Mat & frame)
{
	long long frame_process;
	if (buf_update == 0)
		return false;
	buf_lock.lock();
	frame = mat_buf;
	buf_update = 0;
	frame_process = frame_get;
	camera_recv = camera_recv_buf;
	buf_lock.unlock();
	LOG(DBG_LEVEL, "frame %d begin process", frame_process);
	return true;
}
#endif

bool MattingFifo::get(Mat & frame)
{
	long long frame_process;
	if (buf_update == 0)
		return false;
	buf_lock.lock();
	frame = mat_buf.clone();
	buf_update = 0;
	frame_process = frame_get;
	buf_lock.unlock();
	LOG(DBG_LEVEL, "frame %d begin process", frame_process);
	return true;
}

MattingFifo * matting_fifo;

void process_camera_img_thread()
{
	while (1) {
		Mat frame;
#ifdef DEBUG_DZH
		if (matting_fifo->get2(frame))
#else
		if (matting_fifo->get(frame))
#endif
			matting->process(frame);
		else {
			chrono::milliseconds dura(1);
			this_thread::sleep_for(dura);
		}
	}
}


thread * produce;
thread * check_tune;
#ifdef _USRDLL
#if HAVE_CAMERA==1
bool start_camera_capture();
#endif
#endif
void init_produce_thread(int mode, int produce_rate_)
{
	
	if (mode < 0) {
		matting_fifo = new MattingFifo();
		produce = new thread(process_camera_img_thread);
#ifdef _USRDLL
#if HAVE_CAMERA==1
		start_camera_capture();
#endif
#endif
	}
	else {
		rounds = mode;
		produce = new thread(process_disk_img_thread);
	}
	produce_rate = produce_rate_;

	check_tune = new thread(check_tune_para_thread);
}
#ifdef DEBUG_DZH
bool start_camera_capture();
void init_produce_thread2(int mode, int produce_rate_)
{

	if (mode < 0) {
		matting_fifo = new MattingFifo();
		produce = new thread(process_camera_img_thread);
		start_camera_capture();
	}
	else {
		rounds = mode;
		produce = new thread(process_disk_img_thread);
	}
	produce_rate = produce_rate_;

	check_tune = new thread(check_tune_para_thread);
}
#endif


void wait_produce_thread_end()
{
	produce->join();
	delete produce;
}


#ifndef _USRDLL

#define WIDE 512
#define HEIGHT 384

void self_test()
{
	Mat frame_rgb;
	double start, read_disk_total=0, process_total=0, write_disk_total=0;
	int frame_num = 0;
	Color32 tex[WIDE * HEIGHT];
	
	img_idx = frame_start;
	matting->tune_parameter(init_value);
	while (1) {
		start = (double)getTickCount();
		frame_num++;
		if (!get_nextimg(frame_rgb))
			break;
		double read_disk = ((double)getTickCount() - start) / getTickFrequency();
		read_disk_total += read_disk;
		matting->process(frame_rgb);
		double process = ((double)getTickCount() - start) / getTickFrequency() - read_disk;
		process_total += process;
		if (matting->get_out(tex))
			write_texture(tex, WIDE, HEIGHT);
		double write_disk = ((double)getTickCount() - start) / getTickFrequency() - read_disk - process;
		write_disk_total += write_disk;
	}
		
	cout << "average read disk time:" << read_disk_total / frame_num << '\n';
	cout << "average process time:" << process_total / frame_num << '\n';
	cout << "average write disk time:" << write_disk_total / frame_num << '\n';
#if HAVE_GPU==1 && PROFILE==1
	MattingGPU * matting_gpu = dynamic_cast<MattingGPU *>(matting);
	if (matting_gpu != NULL) {
		cout << "average wait time" << matting_gpu->cpu_wait / frame_num << '\n';
		cout << "average launch time" << matting_gpu->gpu_launch / frame_num << '\n';
		cout << "average process time" << matting_gpu->cpu_process / frame_num << '\n';
	}
#endif
}

//void self_test2()
//{
//	Mat frame;	
//	int frame_num = 0;
//	MattingCPU * matting_cpu = new MattingCPU();
//	MattingCPUOrg * matting_org = new MattingCPUOrg();
//
//	img_idx = frame_start;
//	matting_cpu->reset(WIDE, HEIGHT);
//	matting_org->reset(WIDE, HEIGHT);
//	while (1) {		
//		frame_num++;
//		if (!get_nextimg(frame))
//			break;
//		matting_cpu->process(frame);		
//		matting_org->process(frame);		
//	}
//}

void do_consume()
{
	Color32 tex[WIDE * HEIGHT];
	chrono::milliseconds dura(20);
	int count = 0;

	while (1) {
		if (matting->get_out(tex)) {
			write_texture(tex, WIDE, HEIGHT);
			count = 0;
		}
		else
			count++;
		if (count > 200)
			break;
		this_thread::sleep_for(dura);
	}
}
unsigned char* out;
void test_sth()
{
	Mat A(3, 3, CV_8U);
	A.setTo(128);
	A.at<unsigned char>(0, 0) = 0;
	A.at<unsigned char>(0, 1) = 0;
	A.at<unsigned char>(1, 0) = 0;
	Mat B(3, 3, CV_8U);
	B.setTo(0);
	B.at<unsigned char>(0, 0) = 255;
	B.at<unsigned char>(0, 1) = 255;
	B.at<unsigned char>(1, 0) = 255;
	B.at<unsigned char>(1, 1) = 255;
	Mat C;
	C = A&B;
	cout << A << endl;
	cout << B << endl;
	cout << C << endl; 
	system("pause"); return;


	//Mat alpha(src.rows, src.cols, CV_8UC1);
	//alpha.setTo(255);
	//Mat in[] = { src, alpha };
	//Mat dst(src.rows, src.cols, CV_8UC4, (uchar*)out, CV_MAT_CONT_FLAG);
	//imwrite("argb1.png", dst);
	//cout << 1276 * 1024 * 4 << "     " << src.cols * src.rows * 4 << "     " << endl;
	////int from_to[] = { 0, 0, 1, 3, 2, 2, 3, 1 };
	//int from_to[] = { 0, 0, 1, 1, 2, 2, 3, 3 };
	//mixChannels(in, 2, &dst, 1, from_to, 4);
	//imwrite("argb4.png", dst);
	//return;
}
int main()
{
	//test_sth(); return 0;
	init_log();
#if (HAVE_GPU==1&&USE_GPU==1)
	checkCudaErrors(cudaSetDevice(0));
	cudaProfilerStart();
	matting = new MattingGPU();
#else
	matting = new MattingCPU();
#endif
	//bool do_self_test = false;
	double time = (double)getTickCount();
	matting->reset(WIDE, HEIGHT);	
	if (!do_self_test) {
#ifdef DEBUG_DZH
		init_produce_thread2(-1, 40);
		wait_produce_thread_end();
#else
		init_produce_thread(1, 40);
		do_consume();
		wait_produce_thread_end();
#endif
	}
	else
		self_test();
	time = ((double)getTickCount() - time) / getTickFrequency();
#if HAVE_GPU==1	
	cudaProfilerStop();	
#endif
	cout << "duration " << time;
	cout << ",lost" << matting->get_lost() <<", noupdate"<< matting->get_no_update();
	getchar();
	return 0;
}

#endif