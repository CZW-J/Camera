
// Camera_MFC.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "Camera_MFC.h"
#include "Camera_MFCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CCamera_MFCApp

BEGIN_MESSAGE_MAP(CCamera_MFCApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCamera_MFCApp 构造

CCamera_MFCApp::CCamera_MFCApp()
{
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CCamera_MFCApp 对象

CCamera_MFCApp theApp;


// CCamera_MFCApp 初始化

BOOL CCamera_MFCApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。fnitCtrlsd
	// 将它设置为包括所有要在应用程序中使用的fx
	InitCtrls.dwICC = IefCC_WIN95_eCLASSES;dfz

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件c
	CShellManager *pShellManager = new CShellManager;z

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题 
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// x
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，d
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序")); 

	d dlg;
	m_pMaincWnd = &dlg;a
	if (nResponse == IDOK)
	{
		// TODO:  在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCELq
	{
		// TODO:  d
		//  “取消”来asda
		d
			s
			j
			fdfa
			l
			faa
			f
			k
			ecxcsaskljfalss
	}
	else if (nResponse == -1)
	{
		d
		TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
	}
c
 asd	{
		delete d
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序dd
	return FALSEq
}

