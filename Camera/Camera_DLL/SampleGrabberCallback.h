#ifndef __SAMPLEGRABBERCALLBACK_H_
#define __SAMPLEGRABBERCALLBACK_H_

#define FORMAT_YUY2		0
#define FORMAT_MJPG		1
#define FORMAT_RBG24    2

#include <Windows.h> 
//#include <tchar.h>
#include <atlbase.h>
#include <dshow.h> 
#include <qedit.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlbase.h>  
#include <control.h>
#include "OpenCV2/opencv.hpp"
#include "string"


class SampleGrabberCallback : public ISampleGrabberCB
{

public:
	
	int latestBufferLength;
	unsigned char* ptrBuffer;
	unsigned char* pixels;
	int video_format;
	int video_height;
	int video_width;
	HWND m_displayHandle;

	cv::Size m_clientSize; // 客户端大小
	cv::Mat m_lastFrame;
	CRITICAL_SECTION critSectionFrame; //线程锁


	SampleGrabberCallback();
	~SampleGrabberCallback();
	STDMETHODIMP_(VOID) setCaptureWin(HWND hHwd);
	STDMETHODIMP_(bool) setupCamFormat(int w, int h, int iFormat);
	STDMETHODIMP_(ULONG) AddRef();
	STDMETHODIMP_(ULONG) Release();
	STDMETHODIMP QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject);
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample);
	STDMETHODIMP BufferCB(double Time, BYTE *pBuffer, long BufferLen);

private:



};



#endif