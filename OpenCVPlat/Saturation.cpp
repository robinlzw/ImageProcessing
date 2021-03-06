// Saturation.cpp : 实现文件
//

#include "stdafx.h"
#include "OpenCVPlat.h"
#include "Saturation.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "OpenCVPlatDoc.h"
#include "OpenCVPlatView.h"

// CSaturation 对话框

IMPLEMENT_DYNAMIC(CSaturation, CDialogEx)

CSaturation::CSaturation(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSaturation::IDD, pParent)
{
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	if (pDoc->image.channels() == 3)
	{
		labImg = cv::Mat(pDoc->image.size(), CV_8UC3);
		cv::cvtColor(pDoc->image, labImg, CV_BGR2HSV);
	}
	else
	{
		labImg = pDoc->image.clone();
	}
	old_labImg = labImg.clone();
}

CSaturation::~CSaturation()
{
}

void CSaturation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSaturation, CDialogEx)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CSaturation 消息处理程序


BOOL CSaturation::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SATURATION);
	pSlidCtrl->SetRange(0, 200);
	pSlidCtrl->SetPos(50);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CSaturation::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_SATURATION);
	double value = pSlidCtrl->GetPos() / 50.0;
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	COpenCVPlatView *pView = (COpenCVPlatView*)pMain->GetActiveView();
	int height = labImg.size().height;
	int width = labImg.size().width;
	for (int row = 0; row < height; row++)
	{
		uchar *old_data = old_labImg.ptr<uchar>(row);
		uchar *data = labImg.ptr<uchar>(row);
		for (int col = 0; col < width; col++)
		{
			if (pDoc->image.channels() == 3)
			{
				int t = old_data[col * 3 + 1] * value;
				data[col * 3 + 1] = t > 255 ? 255 : t;
			}
			else
			{
				int t = old_data[col] * value;
				data[col] = t > 255 ? 255 : t;
			}
		}
	}
	if (pDoc->image.channels() == 3)
		cv::cvtColor(labImg, pDoc->image, CV_HSV2BGR);
	else
		pDoc->image = labImg;

	pView->Invalidate();
}
