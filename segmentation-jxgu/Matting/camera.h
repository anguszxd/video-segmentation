
enum CameraType//��Ƶ���뷽ʽ
{
	IPCB = 0,//IP�Ļص���ʽ
	IPSTREAM,//IP������ʽ
	USBCAP,//ϵͳĬ������ͷ
	IMAGE,//ͼƬ���з���
	VIDEO//��Ƶ�ļ�����
};

bool start_camera_capture();

void stop_camera_capture();