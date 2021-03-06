
// Camera_MFCDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

// CCamera_MFCDlg 对话框
class CCamera_MFCDlg : public CDialogEx
{
// 构造
public:
	CCamera_MFCDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CAMERA_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOpencamera();
	afx_msg void OnBnClickedClosecamera();
	CStatic m_pic;
	afx_msg void OnClose();
	afx_msg void OnBnClickedCreateimage();
};
