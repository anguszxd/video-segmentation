#ifndef _GPUOPT_H_
#define _GPUOPT_H_
#define HAVE_GPU 1
#define HAVE_CAMERA 1
#if HAVE_GPU==1
#include <cuda_runtime.h>
#include <cuda_profiler_api.h>
#include "helper_cuda.h"
#endif
#endif
