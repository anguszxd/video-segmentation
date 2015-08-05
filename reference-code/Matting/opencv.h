#ifdef _DEBUG
#pragma comment (lib, "opencv_highgui249d.lib")
#pragma comment (lib, "opencv_core249d.lib")
#pragma comment (lib, "opencv_imgproc249d.lib")
#pragma comment (lib, "opencv_objdetect249d.lib")
#else
#pragma comment (lib, "opencv_highgui249.lib")
#pragma comment (lib, "opencv_core249.lib")
#pragma comment (lib, "opencv_imgproc249.lib")
#pragma comment (lib, "opencv_objdetect249.lib")
#pragma comment (lib, "opencv_gpu249.lib")
#endif

#define XB(x) x+x+x
#define XG(x) x+x+x+1
#define XR(x) x+x+x+2