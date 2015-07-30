#include <thread>
#include <opencv2/opencv.hpp>
#include "../matting/matting.h"
#include <Jai_Factory.h>
#include "camera.h"

extern MattingFifo * matting_fifo;

#ifdef _DEBUG
#pragma comment (lib, "opencv_highgui249d.lib")
#pragma comment (lib, "opencv_core249d.lib")
#pragma comment (lib, "opencv_imgproc249d.lib")
#pragma comment (lib, "opencv_objdetect249d.lib")
#pragma comment (lib, "Jai_Factory.lib")
#else
#pragma comment (lib, "opencv_highgui249.lib")
#pragma comment (lib, "opencv_core249.lib")
#pragma comment (lib, "opencv_imgproc249.lib")
#pragma comment (lib, "opencv_objdetect249.lib")
#pragma comment (lib, "Jai_Factory.lib")
#endif

using namespace std;
using namespace cv;

//#define CAPTYPE IPCB
int cap_type = USBCAP;


#define NODE_NAME_WIDTH         "Width"
#define NODE_NAME_HEIGHT        "Height"
#define NODE_NAME_PIXELFORMAT   "PixelFormat"
#define NODE_NAME_GAIN          "GainRaw"
#define NODE_NAME_ACQSTART      "AcquisitionStart"
#define NODE_NAME_ACQSTOP       "AcquisitionStop"
#define	NUM_OF_BUFFER	5
#define NODE_NAME_FRAMERATEENABLE		"AcquisitionFrameRateEnable"
#define NODE_NAME_FRAMERATEABS			"AcquisitionFrameRateAbs"
#define NODE_NAME_EXPOSURETIMERAW		"ExposureTimeRaw"

uint32_t iRGain = 4096;
uint32_t iGGain = 4096;
uint32_t iBGain = 4096;



FACTORY_HANDLE  m_hFactory;     // Factory Handle
CAM_HANDLE      m_hCam;         // Camera Handle
VIEW_HANDLE     m_hView;        // View window handle
int8_t          m_sCameraId[J_CAMERA_ID_SIZE];    // Camera ID string
int32_t         m_iStreamChannel;   // Stream channel number (default 0)
STREAM_HANDLE   m_hDS;              // Handle to the data stream
HANDLE          m_hStreamThread;    // Handle to the image acquisition thread
HANDLE          m_hStreamEvent;     // Thread used to signal when image thread stops
uint32_t        m_iValidBuffers;    // Number of buffers allocated to image acquisition
uint8_t*        m_pAquBuffer[NUM_OF_BUFFER];    // Buffers allocated to hold image data
BUF_HANDLE      m_pAquBufferID[NUM_OF_BUFFER];  // Handles for all the image buffers
HANDLE	        m_hCondition;       // Condition used for getting the New Image Events
bool            m_bEnableThread;    // Flag indicating if the image thread should run
bool            m_bStreamStarted;   // Flag indicating that the J_Stream_StartAcquisition() has been called
HANDLE          m_hFaceThread;    // Handle to the image acquisition thread
bool            m_bFaceThreadEnable;    // Flag indicating if the image thread should run
HANDLE          m_hMattingThread;
bool            m_bMattingThreadEnable;
J_tIMAGE_INFO tCnvImageInfo;    // Image info structure
IplImage        *m_pImg;        // OpenCV Images


THRD_HANDLE     m_hThread;


void do_preprocess(Mat& frame)
{
	//Rect rc(50, 50, 600, 800);
	//cvWaitKey(10);
	flip(frame, frame, 1);
	//frame = frame(rc);
}

class Camera
{
public:
	virtual bool start_camera_capture()=0;
	virtual void stop_camera_capture()=0;
};


class USBCamera :public Camera
{
	bool thread_on;
	thread t;

	void frame_req()
	{
		VideoCapture video;
		video.open(0);
		if (!video.isOpened())
		{
			cout << "***Could not initialize capturing...***\n";
			return;
		}
		Mat frame;
		while (thread_on)
		{
			video >> frame;
			if (frame.empty())
			{

				thread_on = false;
				break;
			}
			do_preprocess(frame);
			matting_fifo->put(frame);
		}
		video.release();
	}
public:
	bool start_camera_capture()
	{
		thread_on = true;
		t = thread(std::mem_fun(&USBCamera::frame_req), this);
		return true;
	}

	void stop_camera_capture()
	{
		thread_on = false;
		t.join();
	}

};


class IMGCamera :public Camera
{
	thread t;
	bool thread_on;
	int frame_rate;
private:
	bool get_nextimg(Mat & m)
	{
		static string input_path = "C:/chenyu/work/matting/image/save3/";
		static unsigned img_idx = 0;
		char num[20];
		sprintf_s(num, "%04d.bmp", img_idx);
		string file;
		file = input_path + num;
		m = imread(file);
		img_idx++;
		if (img_idx > 536)img_idx = 0;
		if (m.data == NULL)return false;
		return true;
	}

	void get_Image()
	{
		double start;
		while (thread_on)
		{
			start = (double)getTickCount();
			Mat frame;
			get_nextimg(frame);
			//imshow("frame", frame);
			matting_fifo->put(frame);
			double t = ((double)getTickCount() - start) / getTickFrequency() * 1000;
			if (t < 1000 / frame_rate) {
				chrono::milliseconds dura(1000 / frame_rate - (int)t);
				this_thread::sleep_for(dura);
			}
		}
	}
public:

	bool start_camera_capture()
	{
		thread_on = true;
		frame_rate = 40;
		t = thread(std::mem_fun(&IMGCamera::get_Image),this);
		return true;
	}

	void stop_camera_capture()
	{
		thread_on = false;
		t.join();
	}
};

class IPCamera :public Camera
{
public:
	//--------------------------------------------------------------------------------------------------
	// OpenFactoryAndCamera
	//--------------------------------------------------------------------------------------------------
	bool OpenFactoryAndCamera()
	{
		J_STATUS_TYPE   retval;
		uint32_t        iSize;
		uint32_t        iNumDev;
		bool8_t         bHasChange;

		// Open factory
		retval = J_Factory_Open((int8_t*)"", &m_hFactory);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not open factory!"), retval);
			return false;
		}
		//TRACE("Opening factory succeeded\n");

		// Update camera list
		retval = J_Factory_UpdateCameraList(m_hFactory, &bHasChange);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not update camera list!"), retval);
			return false;
		}
		//TRACE("Updating camera list succeeded\n");

		// Get the number of Cameras
		retval = J_Factory_GetNumOfCameras(m_hFactory, &iNumDev);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not get the number of cameras!"), retval);
			return false;
		}
		if (iNumDev == 0)
		{
			//AfxMessageBox(CString("There is no camera!"), MB_OK | MB_ICONEXCLAMATION);
			return false;
		}
		//TRACE("%d cameras were found\n", iNumDev);

		// Get camera ID
		iSize = (uint32_t)sizeof(m_sCameraId);
		retval = J_Factory_GetCameraIDByIndex(m_hFactory, 0, m_sCameraId, &iSize);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not get the camera ID!"), retval);
			return false;
		}
		//TRACE("Camera ID: %s\n", m_sCameraId);

		// Open camera
		retval = J_Camera_Open(m_hFactory, m_sCameraId, &m_hCam);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not open the camera!"), retval);
			return false;
		}
		//TRACE("Opening camera succeeded\n");

		return true;
	}
	//--------------------------------------------------------------------------------------------------
	// Stream2Mat
	//--------------------------------------------------------------------------------------------------
	void StreamCBFunc(J_tIMAGE_INFO * pAqImageInfo)
	{
		static int count = -1;
		// We only want to create the OpenCV Image object once and we want to get the correct size from the Acquisition Info structure
		if (m_pImg == NULL)
		{
			// Allocate the buffer to hold converted the image
			if (J_Image_Malloc(pAqImageInfo, &tCnvImageInfo) != J_ST_SUCCESS)return;

			// Create the Image:
			// We assume this is a 8-bit monochrome image in this sample
			m_pImg = cvCreateImage(cvSize(pAqImageInfo->iSizeX, pAqImageInfo->iSizeY), IPL_DEPTH_8U, 1);
		}
		// Convert the raw image to image format

		// Copy the data from the Acquisition engine image buffer into the OpenCV Image obejct
		memcpy(m_pImg->imageData, pAqImageInfo->pImageBuffer, m_pImg->imageSize);
		Mat frame;
		frame = m_pImg;
		matting_fifo->put(frame);
		count++;
	}

	bool start_camera_capture()
	{

		J_STATUS_TYPE   retval;
		int64_t int64Val;
		int64_t pixelFormat;

		SIZE	ViewSize;
		POINT	TopLeft;

		// Open factory & camera
		retval = OpenFactoryAndCamera();
		if (retval != true)
		{
			// Write error message in textbox
			//GetDlgItem(IDC_CAMERAID)->SetWindowText(CString("Error opening factory and camera"));
			return false;
		}

		// Get Width from the camera based on GenICam name
		retval = J_Camera_GetValueInt64(m_hCam, (int8_t*)NODE_NAME_WIDTH, &int64Val);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not get Width value!"), retval);
			return false;
		}
		ViewSize.cx = (LONG)int64Val;     // Set window size cx

		// Get Height from the camera
		retval = J_Camera_GetValueInt64(m_hCam, (int8_t*)NODE_NAME_HEIGHT, &int64Val);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not get Height value!"), retval);
			return false;
		}
		ViewSize.cy = (LONG)int64Val;     // Set window size cy

		// Get pixelformat from the camera
		retval = J_Camera_GetValueInt64(m_hCam, (int8_t*)NODE_NAME_PIXELFORMAT, &int64Val);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not get PixelFormat value!"), retval);
			return false;
		}
		pixelFormat = int64Val;

		// Calculate number of bits (not bytes) per pixel using macro
		int bpp = J_BitsPerPixel(pixelFormat);

		// Set window position
		TopLeft.x = 100;
		TopLeft.y = 50;

		// Open view window
		/*retval = J_Image_OpenViewWindow(_T("Image View Window"), &TopLeft, &ViewSize, &m_hView);
		if (retval != J_ST_SUCCESS) {
		ShowErrorMsg(CString("Could not open view window!"), retval);
		return;
		}
		TRACE("Opening view window succeeded\n");*/

		// Open stream
		retval = J_Image_OpenStream(m_hCam, 0, reinterpret_cast<J_IMG_CALLBACK_OBJECT>(this), reinterpret_cast<J_IMG_CALLBACK_FUNCTION>(&IPCamera::StreamCBFunc), &m_hThread, (ViewSize.cx*ViewSize.cy*bpp) / 8);
		if (retval != J_ST_SUCCESS) {
			//ShowErrorMsg(CString("Could not open stream!"), retval);
			return false;
		}
		//TRACE("Opening stream succeeded\n");

		// Start Acquision
		retval = J_Camera_ExecuteCommand(m_hCam, (int8_t*)NODE_NAME_ACQSTART);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not Start Acquisition!"), retval);
			return false;
		}

		//TRACE("Opening stream succeeded\n");

		// Start Acquision
		retval = J_Camera_ExecuteCommand(m_hCam, (int8_t*)NODE_NAME_ACQSTART);
		if (retval != J_ST_SUCCESS)
		{
			//ShowErrorMsg(CString("Could not Start Acquisition!"), retval);
			return false;
		}

		return true;

	}
	//--------------------------------------------------------------------------------------------------
	// CloseFactoryAndCamera
	//--------------------------------------------------------------------------------------------------
	void CloseFactoryAndCamera()
	{
		J_STATUS_TYPE   retval;
		// Any camera open?
		if (m_hCam)
		{
			// Close camera
			retval = J_Camera_Close(m_hCam);
			if (retval != J_ST_SUCCESS)
			{
				//ShowErrorMsg(CString("Could not close the camera!"), retval);
			}
			m_hCam = NULL;
			//TRACE("Closed camera\n");
		}

		// Factory open?
		if (m_hFactory)
		{
			// Close factory
			retval = J_Factory_Close(m_hFactory);
			if (retval != J_ST_SUCCESS)
			{
				//ShowErrorMsg(CString("Could not close the factory!"), retval);
			}
			m_hFactory = NULL;
			//TRACE("Closed factory\n");
		}
	}
	void stop_camera_capture()
	{
		J_STATUS_TYPE retval;

		// Start Acquision
		if (m_hCam) {
			retval = J_Camera_ExecuteCommand(m_hCam, (int8_t*)NODE_NAME_ACQSTOP);
			if (retval != J_ST_SUCCESS)
			{
				//ShowErrorMsg(CString("Could not Stop Acquisition!"), retval);
			}
		}

		CloseFactoryAndCamera();
		//TRACE("Closed stream\n");
	}

};

Camera* camera = NULL;
bool start_camera_capture()
{
	if (camera == NULL)
	{
		if (cap_type == IPCB)
		{
			camera = new IPCamera;
		}
		else if (cap_type == IMAGE)
		{
			camera = new IMGCamera;
		}
		else if (cap_type == USBCAP)
		{
			camera = new USBCamera;
		}
		return camera->start_camera_capture();
	}
	return true;
}

void stop_camera_capture()
{
	if (camera != NULL)
	{
		camera->stop_camera_capture();
		delete camera;
		camera = NULL;
	}
}

int _main()
{
	m_pImg = NULL;
	bool b = start_camera_capture();
	system("pause");
	stop_camera_capture();
	return 0;
}