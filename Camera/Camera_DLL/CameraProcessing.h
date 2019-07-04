#pragma once

#ifndef __CAMERAPROCESSING_H__
#define  __CAMERAPROCESSING_H__



#include "SampleGrabberCallback.h"

typedef struct _DeviceCam
{
	char devName[20];
	int num;
	struct _DeviceCam* nextCam;
	_DeviceCam()
	{
		num = 0;
		nextCam = NULL;
	}
}DeviceCam;

class CameraProcessing
{
public:
	CameraProcessing();
	~CameraProcessing();

	int enumCamera(DeviceCam* camInfo);

	int openCam(int device);

	int stopCam();

	bool isOpened();

	int setCamDisplay(int w, int h, int iFormat);

	int previewCam(HWND hwnd);


	SampleGrabberCallback *m_SampleGrabberCB;
private:
	CComPtr<IMediaControl>m_pMC = NULL;				// 媒体控制接口  
	CComPtr<ICaptureGraphBuilder2>m_pCapGB = NULL;    // 增强型捕获滤波器链表管理  
	CComPtr<IGraphBuilder>m_pGB = NULL;				// 滤波链表管理器  
	CComPtr<IBaseFilter> m_pVideoCap = NULL;			// 视频捕获滤波器  
	CComPtr<ISampleGrabber> m_pGrabber = NULL;
	CComPtr<IAMStreamConfig> pConfig = NULL;

	HRESULT InitCaptureGraphBuilder();
	bool BindFilter(int iDeviceID, IBaseFilter** pFilter);
	volatile bool m_isOpened;

};


#endif