#include "stdafx.h"
#include "CameraProcessing.h"
#include <string>
 


using namespace std;



CameraProcessing::CameraProcessing()
{
	m_isOpened = false;
	m_pMC = NULL;				// 媒体控制接口  
	m_pCapGB = NULL;    // 增强型捕获滤波器链表管理  
	m_pGB = NULL;				// 滤波链表管理器  
	m_pVideoCap = NULL;			// 视频捕获滤波器
	m_pGrabber = NULL;
	pConfig = NULL;
}


CameraProcessing::~CameraProcessing()
{
	m_isOpened = false;
	m_pMC = NULL;				// 媒体控制接口  
	m_pCapGB = NULL;    // 增强型捕获滤波器链表管理  
	m_pGB = NULL;				// 滤波链表管理器  
	m_pVideoCap = NULL;			// 视频捕获滤波器
	m_pGrabber = NULL;
	pConfig = NULL;
	 
}



void MyFreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0) {
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL) {
		// Unecessary because pUnk should not be used, but safest.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}


void convertOLEString(LPOLESTR src, string & dest)
{
	USES_CONVERSION;
	dest = string(OLE2CA(src));
}

int CameraProcessing::enumCamera(DeviceCam* camInfo)
{
	int iCount = 0;
	CoInitialize(NULL);
	CComPtr<ICreateDevEnum> pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (FAILED(hr)) {
		CoUninitialize();
		return hr;
	}
	CComPtr<IEnumMoniker> pEm;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,
		&pEm, 0);
	if (FAILED(hr) || pEm == NULL) {
		pCreateDevEnum = NULL;
		CoUninitialize();
		return hr;
	}
 
	pEm->Reset();
	ULONG cFetched;
	IMoniker *pM;
	DeviceCam *g_CamHead = camInfo;

	while (hr = pEm->Next(1, &pM, &cFetched), hr == S_OK) {
		IPropertyBag *pBag = 0;
		hr = pM->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pBag);
		if (FAILED(hr)) {
			pCreateDevEnum = NULL;
			pM->Release();
			pEm = NULL;
			CoUninitialize();
			return hr;
		}
		VARIANT var;
		var.vt = VT_BSTR;
		hr = pBag->Read(L"FriendlyName", &var, NULL);
		if (FAILED(hr)) {
			pCreateDevEnum = NULL;
			pM->Release();
			pEm = NULL;
			CoUninitialize();
			return hr;
		}
		SysFreeString(var.bstrVal);
		pBag->Release();
		LPOLESTR oleString = NULL;
		hr = pM->GetDisplayName(NULL, NULL, &oleString);
		if (FAILED(hr)) {
			pCreateDevEnum = NULL;
			pM->Release();
			pEm = NULL;
			CoUninitialize();
			return hr;
		}
		pM->Release();
		string vendorString;
		convertOLEString(var.bstrVal, vendorString);
		CoTaskMemFree(oleString);
		iCount++;
		if (iCount == 1)
		{
			g_CamHead->num = iCount;
			memset(g_CamHead->devName, 0, sizeof(g_CamHead->devName));
			strcpy(g_CamHead->devName, vendorString.c_str());
		}
		else
		{
			DeviceCam *g_CamNow = new DeviceCam;
			g_CamHead->nextCam = g_CamNow;
			g_CamHead = g_CamNow;
			g_CamHead->num = iCount;
			memset(g_CamHead->devName, 0, sizeof(g_CamHead->devName));
			strcpy(g_CamHead->devName, vendorString.c_str());
		}
	

	}
 
	pCreateDevEnum = NULL;
	pEm = NULL;
	CoUninitialize();
	return S_OK;
 
}

HRESULT CameraProcessing::InitCaptureGraphBuilder()
{
	HRESULT hr;
	hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC,IID_IGraphBuilder,(void **)&m_pGB);
	if (FAILED(hr))
	{
		return hr;
	}
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (void **)&m_pCapGB);
	if (FAILED(hr))
	{
		return hr;
	}

	m_pCapGB->SetFiltergraph(m_pGB);
	hr = m_pGB->QueryInterface(IID_IMediaControl, (void**)&m_pMC);
	if (FAILED(hr))
	{
		return hr;
	}
	return hr;
}

bool CameraProcessing::BindFilter(int iDeviceID, IBaseFilter** pFilter)
{
	if (iDeviceID < 0)
	{
		return false;
	}
	CComPtr<ICreateDevEnum>pCreateDevEnum;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pCreateDevEnum);
	if (FAILED(hr))
	{
		return false;
	}
	CComPtr<IEnumMoniker>pEM;
	hr = pCreateDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEM, 0);
	if (FAILED(hr))
	{
		return false;
	}
	pEM->Reset();

	ULONG cFetched;
	IMoniker *pM;
	int indexDev=0;

	while (hr = pEM->Next(1,&pM,&cFetched),hr == S_OK ,indexDev <= iDeviceID)
	{
		if (indexDev == iDeviceID)
		{
			hr = pM->BindToObject(0, 0, IID_IBaseFilter, (void**)pFilter);
			if (SUCCEEDED(hr))
			{
				pM->Release();
				return true;
			}
			else
			{
				pM->Release();
				return false;
			}
		}
		pM->Release();
		indexDev++;
	}
	return true;
}

 
int CameraProcessing::openCam(int device)
{
	HRESULT hr;
	if (m_isOpened)  return -1;
	hr = InitCaptureGraphBuilder();
	if (FAILED(hr))
	{
		return -1;
	}

	if ( BindFilter(device,&m_pVideoCap)== true)
    {
		hr = m_pGB->AddFilter(m_pVideoCap, L"Video Capture Filter");
		if (FAILED(hr))
		{
			return -1;
		}
    }

	if (m_pVideoCap == NULL)
	{
		return -1;
	}

	CComPtr<IEnumPins> pEnum;
	m_pVideoCap->EnumPins(&pEnum);
	hr = pEnum->Reset();
	if (FAILED(hr))
	{
		return -1;
	}

	CComPtr<IPin> pCameraOutput;
	hr = pEnum->Next(1, &pCameraOutput, NULL);
	if (FAILED(hr))
	{
		return -1;
	}

	int format = 0;
	hr = pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);
	if (FAILED(hr))
	{
		return -1;
	}

	CComPtr<IBaseFilter> pSG_Filter;
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSG_Filter);
	if (FAILED(hr))
	{
		return -1;
	}

	hr = m_pGB->AddFilter(pSG_Filter, L"SampleGrab");
	if (FAILED(hr))
	{
		return -1;
	}
	CComPtr<IBaseFilter> pNull;
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pNull);
	if (FAILED(hr)) {
		return -1;
	}
	hr = m_pGB->AddFilter(pNull, L"NullRender");
	if (FAILED(hr)) {
		return -1;
	}

	hr = m_pCapGB->RenderStream(
		&PIN_CATEGORY_CAPTURE,//&PIN_CATEGORY_CAPTURE, // Connect this pin ...
		&MEDIATYPE_Video, // with this media type ...
		m_pVideoCap, // on this filter ...
		pSG_Filter, // to the Sample Grabber ...
		pNull); // ... and finally to the Null Renderer.
	if (FAILED(hr)) {
		return -1;
	}

	hr = pSG_Filter->QueryInterface(IID_ISampleGrabber, (void**)&m_pGrabber);
	if (FAILED(hr)) {
		return -1;
	}

	hr = m_pGrabber->SetOneShot(FALSE);
	if (FAILED(hr)) {
		return -1;
	}

	hr = m_pGrabber->SetBufferSamples(FALSE);
	if (FAILED(hr)) {
		return -1;
	}

	//设置回调
	m_SampleGrabberCB = new SampleGrabberCallback();
	hr = m_pGrabber->SetCallback(m_SampleGrabberCB, 0);
	if (FAILED(hr)) 
	{
		return -1;
	}
	m_isOpened = true;
	return 0;
}

int CameraProcessing::setCamDisplay(int w, int h, int iFormat)
{
	int icount, isize;
	int x = 0;

	if (m_pMC == NULL) return -1;
	OAFilterState pfs;
	HRESULT hr = m_pMC->GetState(100, &pfs);
	if (FAILED(hr)) {
		return hr;
	}
	if (SUCCEEDED(hr) || pfs == 2) {
		m_pMC->Stop();
	}

	AM_MEDIA_TYPE videoType;
	if (m_pGrabber == NULL)
		return -1;


	hr = m_pGrabber->GetConnectedMediaType(&videoType);
	if (FAILED(hr)) {
		return -1;
	}

	VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*)(videoType.pbFormat);


	if (pConfig == NULL)
		return -1;
	hr = pConfig->GetNumberOfCapabilities(&icount, &isize);
	if (isize != sizeof(VIDEO_STREAM_CONFIG_CAPS))
	{
		return 0;
	}

	VIDEO_STREAM_CONFIG_CAPS scc;
	AM_MEDIA_TYPE *pmtConfig;
	hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);


	pVih->bmiHeader.biWidth = w;
	pVih->bmiHeader.biHeight = h;
	videoType.formattype = FORMAT_VideoInfo;
	videoType.majortype = MEDIATYPE_Video;
 
	if (pmtConfig->subtype == MEDIASUBTYPE_YUY2)
	{
		iFormat = FORMAT_YUY2;
	}
	else if (pmtConfig->subtype == MEDIASUBTYPE_MJPG)
	{
		iFormat = FORMAT_MJPG;
	}
	else if (pmtConfig->subtype == MEDIASUBTYPE_RGB24)
	{
		iFormat = FORMAT_RBG24;
		videoType.subtype = MEDIASUBTYPE_RGB24;
	}
	else if (pmtConfig->subtype == MEDIASUBTYPE_RGB1)
	{
		MessageBox(NULL, L"视频格式为  MEDIASUBTYPE_RGB1", L"", MB_OK);

	}
	else if (pmtConfig->subtype == MEDIASUBTYPE_RGB32)
	{
		MessageBox(NULL, L"视频格式为  MEDIASUBTYPE_RGB32", L"", MB_OK);
	}
	else if (pmtConfig->subtype == MEDIASUBTYPE_RGB565)
	{
		MessageBox(NULL, L"视频格式为  MEDIASUBTYPE_RGB565", L"", MB_OK);
	}
  
	m_SampleGrabberCB->setupCamFormat(w, h, iFormat);
	 
	hr = m_pGrabber->SetMediaType(&videoType);
	if (FAILED(hr)) {
		MyFreeMediaType(videoType);
		return hr;
	}
	hr = pConfig->SetFormat(&videoType);
	MyFreeMediaType(videoType);

	return 0;
}


int CameraProcessing::previewCam(HWND hwnd)
{
	if (m_isOpened == false) return -1;
	if (m_pMC == NULL || m_SampleGrabberCB == NULL) { return -1; }
	OAFilterState pfs;
	HRESULT hr = m_pMC->GetState(100, &pfs);
	if (FAILED(hr))
	{
		return hr;
	}
	if (SUCCEEDED(hr) || pfs == 2) {
		m_pMC->Stop();
	}
 //	m_CaptureWin = hwnd;
 	m_SampleGrabberCB->setCaptureWin(hwnd);
	hr = m_pMC->Run();
//	m_isStop = false;
	return 0;
}

int CameraProcessing::stopCam()
{
	if (m_isOpened == false) return 0;
	if (m_pMC == NULL)return 0;
	m_pMC->Stop();
	if ((m_SampleGrabberCB) && (m_pGrabber)) {
		m_pGrabber->SetCallback(NULL, 0);
		m_SampleGrabberCB->Release();
		delete m_SampleGrabberCB;
		m_SampleGrabberCB = NULL;
	}
	m_isOpened = false;
}

bool CameraProcessing::isOpened()
{
	return m_isOpened;
}


