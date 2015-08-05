
enum CameraType//视频输入方式
{
	IPCB = 0,//IP的回调方式
	IPSTREAM,//IP的流方式
	USBCAP,//系统默认摄像头
	IMAGE,//图片序列仿真
	VIDEO//视频文件仿真
};

bool start_camera_capture();

void stop_camera_capture();