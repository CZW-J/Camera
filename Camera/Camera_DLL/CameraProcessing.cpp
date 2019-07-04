#include "stdafx.h"
#include "CameraProcessing.h"
#include <string>
 


using namespace std;



CameraProcessing::CameraProcessing()
{

}


CameraProcessing::~CameraProcessing()
{


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
		int nSize = 1024;
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

}


int CameraProcessing::openCam(int device)
{
	HRESULT hr;
	if (m_isOpened)  return -1;
	hr = InitCaptureGraphBuilder();
	if (FAILED(hr))
	{
		return hr;
	}

	if ( BindFilter(device,&m_pVideoCap)== true)
    {
		hr = m_pGB->AddFilter(m_pVideoCap, L"Video Capture Filter");
		if (FAILED(hr))
		{
			return hr;
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
		return hr;
	}

	CComPtr<IPin> pCameraOutput;
	hr = pEnum->Next(1, &pCameraOutput, NULL);
	if (FAILED(hr))
	{
		return hr;
	}

	int format = 0;
	hr = pCameraOutput->QueryInterface(IID_IAMStreamConfig, (void**)&pConfig);
	if (FAILED(hr))
	{
		return hr;
	}

	CComPtr<IBaseFilter> pSG_Filter;
	hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pSG_Filter);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = m_pGB->AddFilter(pSG_Filter, L"SampleGrab");
	if (FAILED(hr))
	{
		return hr;
	}
	CComPtr<IBaseFilter> pNull;
	hr = CoCreateInstance(CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)&pNull);
	if (FAILED(hr)) {
		return hr;
	}
	hr = m_pGB->AddFilter(pNull, L"NullRender");
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_pCapGB->RenderStream(
		&PIN_CATEGORY_CAPTURE,//&PIN_CATEGORY_CAPTURE, // Connect this pin ...
		&MEDIATYPE_Video, // with this media type ...
		m_pVideoCap, // on this filter ...
		pSG_Filter, // to the Sample Grabber ...
		pNull); // ... and finally to the Null Renderer.
	if (FAILED(hr)) {
		return hr;
	}

	hr = pSG_Filter->QueryInterface(IID_ISampleGrabber, (void**)&m_pGrabber);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_pGrabber->SetOneShot(FALSE);
	if (FAILED(hr)) {
		return hr;
	}

	hr = m_pGrabber->SetBufferSamples(FALSE);
	if (FAILED(hr)) {
		return hr;
	}

}

int CameraProcessing::stopCam()
{

}

bool CameraProcessing::isOpened()
{

}