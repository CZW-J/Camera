
// Camera_MFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CCamera_MFCApp: 
// �йش����ʵ�֣������ Camera_MFC.cpp
//

class CCamera_MFCApp : public CWinApp
{
public:
	CCamera_MFCApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CCamera_MFCApp theApp;