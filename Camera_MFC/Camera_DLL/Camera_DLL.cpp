// Camera_DLL.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Camera_DLL.h"
#include "CameraProcessing.h"
#include <string>
using namespace std;
CameraProcessing cameraProcessing;
int m_camNum=0;
char g_buf[1024];

char* WINAPI getCamera()
{
	DeviceCam *camInfo=new DeviceCam;
	cameraProcessing.enumCamera(camInfo);
	string result;
 
	while (camInfo != NULL)
	{
		DeviceCam *tmpCamInfo = camInfo->nextCam;
		m_camNum = camInfo->num;
		strcpy(g_buf, camInfo->devName); 
		result = result + g_buf;
		delete []camInfo;
		camInfo = tmpCamInfo;
	}
	if (m_camNum > 0)
	{
		return g_buf;
	}
	return NULL;
}

char* WINAPI openCamera(HWND winHwnd ,int resolutionX,int resolutionY,int device)
{
	cameraProcessing.openCam(device);
	return 0;
}

char* WINAPI closeCamera()
{
	return 0;
}
