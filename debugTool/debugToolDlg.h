
// debugToolDlg.h: 標頭檔
//

#pragma once
using namespace std;
#include "CExportLog.h"
#include <vector>

// CdebugToolDlg 對話方塊
class CdebugToolDlg : public CDialogEx
{
// 建構
public:
	CdebugToolDlg(CWnd* pParent = nullptr);	// 標準建構函式

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DEBUGTOOL_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支援


// 程式碼實作
protected:
	HICON m_hIcon;

	// 產生的訊息對應函式
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//UI object and event
	CButton BtnExport;
	CProgressCtrl cpcProgressOfOutputLog;

	//UI event
	afx_msg void OnBnClose();
	afx_msg void OnBnExportLog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);

private:
	CExportLog cel;
};
