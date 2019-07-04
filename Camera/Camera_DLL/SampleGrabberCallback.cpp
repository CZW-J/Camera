#include "stdafx.h"
#include "SampleGrabberCallback.h"


using namespace cv;
using namespace std;
SampleGrabberCallback::SampleGrabberCallback()
{
	InitializeCriticalSection(&critSectionFrame);
}


SampleGrabberCallback::~SampleGrabberCallback()
{
	DeleteCriticalSection(&critSectionFrame);
}

STDMETHODIMP_(VOID) SampleGrabberCallback::setCaptureWin(HWND hHwd)
{
	RECT clientRect;
	GetClientRect(hHwd, &clientRect);
	int win_w, win_h;
	win_w = (clientRect.right - clientRect.left);
	win_h = (clientRect.bottom - clientRect.top);
	win_w = win_w / 4 * 4;
	win_h = win_h / 4 * 4;
  	m_clientSize = cv::Size(win_w, win_h);
  	m_displayHandle = hHwd;

}


STDMETHODIMP_(bool) SampleGrabberCallback::setupCamFormat(int w, int h, int iFormat)
{
	if (video_height != h || video_width != w || video_format != iFormat) {
		if (pixels != NULL) {
			delete pixels;
		}
	}
	
	video_format = iFormat;
	video_width = w;
	video_height = h;
	int numBytes = w * h * 3;
	switch (iFormat)
	{
	case FORMAT_YUY2:
		numBytes = w*h * 2;
		break;
	case FORMAT_MJPG:
		numBytes = w*h ;
		break;
	case FORMAT_RBG24:
		numBytes = w*h * 3;
		break;
	}
	pixels = new unsigned char[numBytes];
	latestBufferLength = 0;
	return true;
}


STDMETHODIMP_(ULONG) SampleGrabberCallback::AddRef()
{
	return 1;
}


STDMETHODIMP_(ULONG) SampleGrabberCallback::Release()
{
	return 2;
}


STDMETHODIMP SampleGrabberCallback::QueryInterface(REFIID riid, _COM_Outptr_ void **ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;
	if (riid == __uuidof(IUnknown)) {
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	if (riid == __uuidof(ISampleGrabberCB)) {
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	return E_NOTIMPL;

}

int yuv2torgb_pixex(int y, int u, int v)
{
	UINT32 pixel32 = 0;
	uchar *pixel = (uchar *)&pixel32;
	int r, g, b;
	r = y + (1.370705 * (v - 128));
	g = y - (0.698001 * (v - 128)) - (0.337633 * (u - 128));
	b = y + (1.732446 * (u - 128));
	if (r > 255) r = 255;
	if (g > 255) g = 255;
	if (b > 255) b = 255;
	if (r < 0) r = 0;
	if (g < 0) g = 0;
	if (b < 0) b = 0;
	pixel[0] = r * 220 / 256;
	pixel[1] = g * 220 / 256;
	pixel[2] = b * 220 / 256;
	return pixel32;
}


int yuv2torgb_buffer(uchar *yuv, uchar *rgb, UINT32 size)
{
	UINT32 in, out = 0;
	UINT32 pixel_16;
	uchar pixel_24[3];
	UINT32 pixel32;
	int y0, u, y1, v, t;
	t = size * 2;
	for (in = 0; in < t; in += 4) {
		pixel_16 =
			yuv[in + 3] << 24 |
			yuv[in + 2] << 16 |
			yuv[in + 1] << 8 |
			yuv[in + 0]; //YUV422每个像素2字节，每两个像素共用一个Cr,Cb值，即u和v，RGB24每个像素3个字节  
		y0 = (pixel_16 & 0x000000ff);
		u = (pixel_16 & 0x0000ff00) >> 8;
		y1 = (pixel_16 & 0x00ff0000) >> 16;
		v = (pixel_16 & 0xff000000) >> 24;
		pixel32 = yuv2torgb_pixex(y0, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2]; //rgb的一个像素  
		pixel32 = yuv2torgb_pixex(y1, u, v);
		pixel_24[0] = (pixel32 & 0x000000ff);
		pixel_24[1] = (pixel32 & 0x0000ff00) >> 8;
		pixel_24[2] = (pixel32 & 0x00ff0000) >> 16;
		rgb[out++] = pixel_24[0];
		rgb[out++] = pixel_24[1];
		rgb[out++] = pixel_24[2];
	}
	return 0;
}

void ImgRotate90(cv::Mat imgIn, int Angle, cv::Mat& imgOut)//逆时针 0,90,180,270
{
	cv::Mat img_rotate = imgIn.clone();

	switch (Angle)
	{
	case 1: transpose(img_rotate, img_rotate); flip(img_rotate, img_rotate, 1); break;
	case 2: flip(img_rotate, img_rotate, -1);  break;
	case 3: transpose(img_rotate, img_rotate); flip(img_rotate, img_rotate, 0); break;
	case 4: flip(img_rotate, img_rotate, 1); break;
	case 5:  transpose(img_rotate, img_rotate); break;
	case 6:  flip(img_rotate, img_rotate, 0); break;
	case 7:  transpose(img_rotate, img_rotate); flip(img_rotate, img_rotate, -1); break;
	default:
		break;
	}
	imgOut = img_rotate;
}

void cvtBufferToMat(BYTE * buffer, int width, int height, int bufSize, int format, cv::Mat& dst)
{
	Mat buf;
	Mat rgb;
	string buf_str;
	vector<uchar> img_data;
	switch (format) {
	case  1:   //FORMAT_MJPG:	
		buf_str = string(buffer, buffer + bufSize);
		img_data = vector<uchar>(buf_str.begin(), buf_str.end());
		dst = imdecode(img_data, CV_LOAD_IMAGE_ANYCOLOR);
		break;

	case 0: // yuv12
		rgb = Mat(height, width, CV_8UC3);
		yuv2torgb_buffer(buffer, rgb.data, width * height);
		cvtColor(rgb, dst, CV_RGB2BGR); // RGB转换为BGR 
		break;
	case 2:  //FORMAT_RGB24	
		dst = Mat(height, width, CV_8UC3, buffer);
		break;
	}
}

void displayFrame(HWND hwd, const cv::Mat& frame)
{
	// 将Mat显示到界面上
	int pixelBytes = frame.channels()*(frame.depth() + 1);
	BITMAPINFO *m_bmphdr;
	DWORD dwBmpHdr = sizeof(BITMAPINFO);
	m_bmphdr = new BITMAPINFO[dwBmpHdr];
	m_bmphdr->bmiHeader.biBitCount = 8 * pixelBytes; // 8 * src.channels();;
	m_bmphdr->bmiHeader.biClrImportant = 0;
	m_bmphdr->bmiHeader.biSize = dwBmpHdr;
	m_bmphdr->bmiHeader.biWidth = frame.cols;
	m_bmphdr->bmiHeader.biHeight = -frame.rows;
	m_bmphdr->bmiHeader.biClrUsed = 0;
	m_bmphdr->bmiHeader.biPlanes = 1;
	m_bmphdr->bmiHeader.biCompression = BI_RGB;

	HDC pHDC = ::GetDC(hwd);
	int nResult = ::StretchDIBits(pHDC,
		0,
		0,
		frame.cols,//rc.right - rc.left,  
		frame.rows,//rc.top,  
		0, 0,
		frame.cols, frame.rows,
		frame.data,
		m_bmphdr,
		DIB_RGB_COLORS,
		SRCCOPY);
	delete[]m_bmphdr;
	ReleaseDC(hwd, pHDC);
}

STDMETHODIMP SampleGrabberCallback::SampleCB(double SampleTime, IMediaSample *pSample)
{
	 
	HRESULT hr = pSample->GetPointer(&ptrBuffer);
	if (hr == S_OK) 
	{
		EnterCriticalSection(&critSectionFrame);
		latestBufferLength = pSample->GetActualDataLength();
		memcpy(pixels, ptrBuffer, latestBufferLength);

		cvtBufferToMat(pixels, 640, 480, latestBufferLength, 0, m_lastFrame);
		if (!m_lastFrame.empty())
		{
			ImgRotate90(m_lastFrame, 0, m_lastFrame);  //翻转
			cv::Mat display_frame;
			cv::resize(m_lastFrame, display_frame, m_clientSize);
			displayFrame(m_displayHandle, display_frame);
			return S_OK;
		}
		LeaveCriticalSection(&critSectionFrame);
	}
	return S_OK;
}

STDMETHODIMP SampleGrabberCallback::BufferCB(double Time, BYTE *pBuffer, long BufferLen)
{
	return S_OK;
}
