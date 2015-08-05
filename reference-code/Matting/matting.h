#ifndef _MATTING_H_
#define _MATTING_H_
#include "Gpuopt.h"
#include <opencv2/opencv.hpp>
#include <mutex>
#include <stdio.h>
#include "opencv.h"

struct Color32 {
	unsigned char r, g, b, a;
};

struct TuningPara {
	int show_content;
	int motion_TH;
	int alpha_TH;
	int static_MAX;
	int init_static_num;
	int luma_offset;
	int u_gain;
	int v_gain;
	int static_NUM;
	int long_static_NUM;
	int alpha_filter_r;
	int block_th;
	float rasie_hands_radius;
	float static_SPEED;
	float long_SPEED;
	float GAMMA;
	float process_Scale;
};

class Matting 
{
public:
	virtual void reset(int out_width_, int out_height_) = 0;
	virtual bool tune_parameter(TuningPara & para) = 0;
	virtual void process(cv::Mat & frame_rgb) = 0;
	virtual int get_out(Color32 * tex) = 0;
	virtual int get_lost() = 0;
	virtual int get_no_update() = 0;	
};

struct TuningParaFloat {
	int show_content;
	float motion_TH_f;
	float alpha_TH_f;
	int static_MAX;
	int init_static_num;
	float luma_offset_f;
	float u_gain_f;
	float v_gain_f;
	int static_NUM;
	int long_static_NUM;
	int alpha_filter_r;
	int block_th;
	float rasie_hands_radius;
	float static_SPEED_f;
	float long_SPEED_f;
	float GAMMA;
	float process_Scale;
};

class MattingCPU : public Matting
{
protected:
	cv::Mat frame_rgb_pre, frame_rgb, bg_yuv;
	cv::Mat	static_num, is_bg, is_bg_raw, fg_sure, alpha_erode, is_body;
	cv::Mat mask, out_mask, out_rgb, out_rgb_buf;
	volatile int out_update, const_updated;
	int out_width, out_height, lost, no_update;
	std::mutex out_lock, const_update_lock;
	long long frame_num;
	TuningParaFloat Const;
	TuningPara const_update;
public:
	MattingCPU();
	void reset(int out_width_, int out_height_);
	bool tune_parameter(TuningPara & para);
	void process(cv::Mat & frame);
	int get_out(Color32 * tex);
	int get_lost();
	int get_no_update();
};

#if HAVE_GPU==1
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/gpu/stream_accessor.hpp>
#define CHANNEL 3
#define PROFILE 0
class MattingGPU : public Matting
{
protected:
	cv::gpu::GpuMat frame_rgb_raw_gpu, frame_rgb_gpu, frame_rgb_pre_gpu, bg_yuv_gpu, static_num_gpu, is_bg_gpu, is_body_gpu, alpha_erode_gpu, box_buf, dilate_buf, erode_buf;
	cv::gpu::CudaMem frame_rgb_cpu, alpha_erode_cpu;
	cv::gpu::CudaMem  bg_yuv_cpu, is_bg_cpu;
	cv::Mat out_mask, out_rgb, out_rgb_buf;
	cv::gpu::Stream stream;
	volatile int out_update, const_updated;
	int out_width, out_height, lost, no_update;
	std::mutex out_lock, const_update_lock;
	long long frame_num;
	TuningPara const_update;
	TuningParaFloat Const;
	cv::Ptr<cv::gpu::FilterEngine_GPU> box_filter;
	cv::Ptr<cv::gpu::FilterEngine_GPU> dilate_filter;
	cv::Ptr<cv::gpu::FilterEngine_GPU> erode_filter;
	cv::gpu::CascadeClassifier_GPU face_detector;
public:
	MattingGPU();
	void reset(int out_width_, int out_height_);
	bool tune_parameter(TuningPara & para);
	void process(cv::Mat & frame);
	int get_out(Color32 * tex);
	int get_lost();
	int get_no_update();
#if PROFILE==1
	double cpu_wait, gpu_launch, cpu_process;
#endif
};
struct HostPara {
	int body_top ;
	int body_bottom;
	int body_left;
	int body_right;
};
#endif


class MattingFifo
{
protected:
	cv::Mat mat_buf;
	std::mutex buf_lock;
	int buf_update;
	int lost;
	long long frame_num, frame_get;
public:
	MattingFifo();
	int get_lost() { return lost; }
#ifdef DEBUG_DZH
	void put2(cv::Mat & frame,double time_recv);
	bool get2(cv::Mat & frame);
#endif
	void put(cv::Mat & frame);
	bool get(cv::Mat & frame);
};


#define DBG_LEVEL 0
#define WARN_LEVEL 1
#define ERR_LEVEL 2
#define LOG_BUFFER 655360
class MattingLog
{
protected:
	double start_time;
	std::mutex lock;	
	bool log_time;
	char * log_buf;
	FILE * log_fp;
	int log_len;
	std::string filename;
public:
	int print_level;
	MattingLog(std::string _filename = "log.txt", int _print_level = DBG_LEVEL, bool _log_time = true);
	~MattingLog();
	double get_time();
	void log(char * s, int len);
	void flush();
};

#ifdef MATTING_C
MattingLog * matting_log;
#else
extern MattingLog * matting_log;
#endif

#define LOG(LEVEL, ...) \
do { \
	if (LEVEL >= matting_log->print_level) { \
		char str[500]; \
		int len =sprintf_s(str, 500, __VA_ARGS__); \
		if (str[len-1]!='\n') { \
			str[len++] ='\n'; \
			str[len] = 0; \
		} \
		matting_log->log(str, len); \
		} \
} while (0)

void init_log();
void init_produce_thread(int mode, int produce_rate_);
void wait_produce_thread_end();
void write_texture(Color32 *tex, int wide, int height);
#endif