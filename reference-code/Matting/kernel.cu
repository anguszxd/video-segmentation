#include "opencv2/gpu/device/common.hpp"
#include <opencv2/core/core.hpp>
using namespace cv::gpu;
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "../Matting/matting.h"
#if HAVE_GPU ==1
#define BLOCK_WIDE 64
#define BLOCK_HIGH 8

#define alpha_top 40
#define alpha_bottom 40
#define alpha_left 120
#define alpha_right 120

namespace cv {
	namespace gpu {
		namespace device {
			/*__device__ const float motion_TH_f = motion_TH / 255.0;
			__device__ const float static_SPEED_f = static_SPEED / 255.0;
			__device__ const float long_SPEED_f = long_SPEED / 255.0;
			__device__ const float luma_offset_f = luma_offset / 255.0f;
			__device__ const float u_gain_f = u_gain;
			__device__ const float v_gain_f = v_gain;*/
			__constant__ TuningParaFloat Const;
			__constant__ HostPara host_para;
			__global__  void trace_bg_kernel(PtrStepSz<float> motion_diff_rgb_filted0, PtrStepSz<float> motion_diff_rgb_filted1, PtrStepSz<float> motion_diff_rgb_filted2, 
					PtrStepSz<float3> frame_yuv, PtrStepSz<float3> bg_yuv, PtrStepSz<float3> bg_diff_yuv, PtrStepSzb static_num, PtrStepSzb is_bg, PtrStepSzb is_body)
			{
				extern __shared__ float smem[];
				typename float * gray = smem;
				unsigned int gray_idx = threadIdx.y * blockDim.x + threadIdx.x;
				unsigned int y = blockIdx.y * (blockDim.y-2) + threadIdx.y;
				unsigned int x = blockIdx.x * (blockDim.x-2) + threadIdx.x;
				
				if (y < static_num.rows && x < static_num.cols) {
					gray[gray_idx] = frame_yuv.ptr(y)[x].x;
					__syncthreads();
					if (threadIdx.x != 0 && threadIdx.y != 0 && threadIdx.x != blockDim.x - 1 && threadIdx.y != blockDim.y - 1 
						&& y + 1<static_num.rows && x + 1<static_num.cols) {
						float edge_offset = MAX(fabs(gray[gray_idx - blockDim.x - 1] - gray[gray_idx + blockDim.x + 1]),
							fabs(gray[gray_idx - blockDim.x + 1] - gray[gray_idx + blockDim.x - 1])) / 2;
						float motion_diff = fabs(motion_diff_rgb_filted0.ptr(y)[x]) + fabs(motion_diff_rgb_filted1.ptr(y)[x]) + fabs(motion_diff_rgb_filted2.ptr(y)[x]);
						unsigned char static_num_reg = static_num.ptr(y)[x];
						if (motion_diff < edge_offset + Const.motion_TH_f)
							static_num_reg = MIN(static_num_reg + 1, Const.static_MAX);
						else
							static_num_reg = 0;
						static_num.ptr(y)[x] = static_num_reg;
						float3 bg_yuv_reg = bg_yuv.ptr(y)[x];
						if (fabs(bg_yuv_reg.x) <= 0.001f && fabs(bg_yuv_reg.y - 1.0f) <= 0.001f && fabs(bg_yuv_reg.z) <=0.001f) {
							if (static_num_reg>= Const.init_static_num) 
								bg_yuv.ptr(y)[x] = frame_yuv.ptr(y)[x];								
						}
						else {
							float update_speed;
							if (is_bg.ptr(y)[x] && static_num_reg >= Const.static_NUM)
								update_speed = Const.static_SPEED_f;
							else if (is_body.ptr(y)[x] == 0 && static_num_reg >= Const.long_static_NUM)
								update_speed = Const.long_SPEED_f;
							else
								update_speed = 0;
							float3 bg_diff_yuv_reg = bg_diff_yuv.ptr(y)[x];
							bg_yuv_reg.x = (bg_diff_yuv_reg.x > 0) ? (bg_yuv_reg.x + update_speed) : (bg_yuv_reg.x - update_speed);
							bg_yuv_reg.y = (bg_diff_yuv_reg.y > 0) ? (bg_yuv_reg.y + update_speed) : (bg_yuv_reg.y - update_speed);
							bg_yuv_reg.z = (bg_diff_yuv_reg.z > 0) ? (bg_yuv_reg.z + update_speed) : (bg_yuv_reg.z - update_speed);
							bg_yuv.ptr(y)[x] = bg_yuv_reg;
						}						
					} 
				}
			}
			
			__global__ void update_mask_bg_kernel(PtrStepSz<float> bg_diff_filted0, PtrStepSz<float> bg_diff_filted1, PtrStepSz<float> bg_diff_filted2, 
					PtrStepSzb fg_sure, PtrStepSzb fg_maybe, PtrStepSzb is_body)
			{			
				unsigned int y = blockIdx.y * blockDim.y + threadIdx.y + alpha_top;
				unsigned int x = blockIdx.x * blockDim.x + threadIdx.x + alpha_left;
				
				float bg_diff_abs_y = fabs(bg_diff_filted0.ptr(y)[x]);
				float bg_diff_abs_u = fabs(bg_diff_filted1.ptr(y)[x]);
				float bg_diff_abs_v = fabs(bg_diff_filted2.ptr(y)[x]);
				
				bg_diff_abs_y = MAX(0.0f, bg_diff_abs_y - Const.luma_offset_f);
				bg_diff_abs_u = bg_diff_abs_u * Const.u_gain_f;
				bg_diff_abs_v = bg_diff_abs_v * Const.v_gain_f;
				float bg_diff_all = (bg_diff_abs_y + bg_diff_abs_u + bg_diff_abs_v)*(fg_sure.ptr(y)[x] + 1);
				float motion_th = Const.alpha_TH_f;
				if ((y >= host_para.body_top - 1) && (y <= host_para.body_bottom - 1) && (x >= host_para.body_left - 1) && (x <= host_para.body_right - 1)) {
					is_body.ptr(y)[x] = 1;
					motion_th = Const.alpha_TH_f / 2;
				} else
					is_body.ptr(y)[x] = 0;

				if (bg_diff_all > motion_th * 2) {					
					fg_sure.ptr(y)[x] = 255;
					fg_maybe.ptr(y)[x] = 255;									
				}
				else {
					fg_sure.ptr(y)[x] = 0;					
					if (bg_diff_all > motion_th)
						fg_maybe.ptr(y)[x] = 255;
					else 
						fg_maybe.ptr(y)[x] = 0;				
				}
			}
			
			void trace_bg_(PtrStepSzb motion_diff_rgb_filted0, PtrStepSzb motion_diff_rgb_filted1, PtrStepSzb motion_diff_rgb_filted2, 
					PtrStepSzb frame_yuv, PtrStepSzb bg_yuv, PtrStepSzb bg_diff_yuv, PtrStepSzb static_num, PtrStepSzb is_bg, PtrStepSzb is_body, cudaStream_t stream)
			{
				const dim3 block(BLOCK_WIDE, BLOCK_HIGH);
				const dim3 grid(divUp(frame_yuv.cols - 2, BLOCK_WIDE - 2), divUp(frame_yuv.rows - 2, BLOCK_HIGH - 2));
				const size_t smemSize = BLOCK_WIDE * BLOCK_HIGH * sizeof(float);
				
				trace_bg_kernel<< <grid, block, smemSize, stream >> > (static_cast<PtrStepSz<float>>(motion_diff_rgb_filted0), static_cast<PtrStepSz<float>>(motion_diff_rgb_filted1), static_cast<PtrStepSz<float>>(motion_diff_rgb_filted2), 
					static_cast<PtrStepSz<float3>>(frame_yuv), static_cast<PtrStepSz<float3>>(bg_yuv), static_cast<PtrStepSz<float3>>(bg_diff_yuv), static_num, is_bg, is_body);
			}
			
			void update_mask_bg_(PtrStepSzb bg_diff_filted0, PtrStepSzb bg_diff_filted1, PtrStepSzb bg_diff_filted2, 
					PtrStepSzb fg_sure, PtrStepSzb fg_maybe, PtrStepSzb is_body, cudaStream_t stream)
			{
				const dim3 block(BLOCK_WIDE, BLOCK_HIGH);
				const dim3 grid(divUp(fg_sure.cols - alpha_left - alpha_right, BLOCK_WIDE), divUp(fg_sure.rows - alpha_top - alpha_bottom, BLOCK_HIGH));
				const size_t smemSize = 0;
				
				update_mask_bg_kernel << <grid, block, smemSize, stream >> > (static_cast<PtrStepSz<float>>(bg_diff_filted0), static_cast<PtrStepSz<float>>(bg_diff_filted1), static_cast<PtrStepSz<float>>(bg_diff_filted2),
					fg_sure, fg_maybe, is_body);
			}
		}
	}
}

void trace_bg(PtrStepSzb motion_diff_rgb_filted0, PtrStepSzb motion_diff_rgb_filted1, PtrStepSzb motion_diff_rgb_filted2, 
					PtrStepSzb frame_yuv, PtrStepSzb bg_yuv, PtrStepSzb bg_diff_yuv, PtrStepSzb static_num, PtrStepSzb is_bg, PtrStepSzb is_body, cudaStream_t stream)
{
	CV_Assert(motion_diff_rgb_filted0.cols==is_bg.cols && frame_yuv.cols==is_bg.cols && bg_yuv.cols==is_bg.cols && bg_diff_yuv.cols==is_bg.cols
		&& static_num.cols==is_bg.cols && is_body.cols==is_bg.cols);
	CV_Assert(motion_diff_rgb_filted0.rows==is_bg.rows && frame_yuv.rows==is_bg.rows && bg_yuv.rows==is_bg.rows && bg_diff_yuv.rows==is_bg.rows
		&& static_num.rows==is_bg.rows && is_body.rows==is_bg.rows);
		
	device::trace_bg_(motion_diff_rgb_filted0, motion_diff_rgb_filted1, motion_diff_rgb_filted2, frame_yuv, bg_yuv, 
		bg_diff_yuv, static_num, is_bg, is_body, stream);
}

void update_mask_bg(PtrStepSzb bg_diff_filted0, PtrStepSzb bg_diff_filted1, PtrStepSzb bg_diff_filted2, 
					PtrStepSzb fg_sure, PtrStepSzb fg_maybe, PtrStepSzb is_body, cudaStream_t stream)
{
	CV_Assert(bg_diff_filted0.cols==is_body.cols && bg_diff_filted1.cols==is_body.cols && bg_diff_filted2.cols==is_body.cols
		&& fg_sure.cols==is_body.cols && fg_maybe.cols==is_body.cols);
	CV_Assert(bg_diff_filted0.rows==is_body.rows && bg_diff_filted1.rows==is_body.rows && bg_diff_filted2.rows==is_body.rows
		&& fg_sure.rows==is_body.rows && fg_maybe.rows==is_body.rows);

	device::update_mask_bg_(bg_diff_filted0, bg_diff_filted1, bg_diff_filted2, fg_sure, fg_maybe, is_body, stream);
}

void tune_gpu_parameter(TuningParaFloat *c)
{
	checkCudaErrors(cudaMemcpyToSymbol(device::Const, c, sizeof(TuningParaFloat)));
}

void update_host_para(HostPara *p)
{
	checkCudaErrors(cudaMemcpyToSymbol(device::host_para, p, sizeof(HostPara)));
}

#endif