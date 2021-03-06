// GaussianBlur.cpp : 实现文件
//

#include "stdafx.h"
#include "OpenCVPlat.h"
#include "GaussianBlur.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "OpenCVPlatDoc.h"
#include "OpenCVPlatView.h"


// CGaussianBlur 对话框

IMPLEMENT_DYNAMIC(CGaussianBlur, CDialogEx)

CGaussianBlur::CGaussianBlur(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGaussianBlur::IDD, pParent)
{
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	old_img = pDoc->image.clone();

}

CGaussianBlur::~CGaussianBlur()
{
}

void CGaussianBlur::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGaussianBlur, CDialogEx)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CGaussianBlur 消息处理程序


BOOL CGaussianBlur::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_GAUSSIAN_BLUR);
	pSlidCtrl->SetRange(0, 10);
	pSlidCtrl->SetTicFreq(1);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_GAUSSIAN_DELTA);
	pSlidCtrl->SetRange(0, 50);
	pSlidCtrl->SetTicFreq(5);
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CGaussianBlur::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_GAUSSIAN_BLUR);
	int value = pSlidCtrl->GetPos();
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_GAUSSIAN_DELTA);
	double delta = pSlidCtrl->GetPos();
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	COpenCVPlatView *pView = (COpenCVPlatView*)pMain->GetActiveView();
	int window_size = 2 * value + 1;
	double *window = new double[window_size * window_size];
	for (int i = 0; i < window_size; i++)
	for (int j = 0; j < window_size; j++)
		window[i * window_size + j] = gaussian_function(i - window_size / 2, j - window_size / 2, delta);
	int height = pDoc->image.size().height;
	int width = pDoc->image.size().width;
	if (pDoc->image.channels() == 3)
	{
		for (int row = 0; row < height; row++)
		{
			uchar *data = pDoc->image.ptr<uchar>(row);
			for (int col = 0; col < width; col++)
			{
				double r = 0, g = 0, b = 0;
				double w = 0;
				for (int i = row - window_size / 2; i <= row + window_size / 2; i++)
				for (int j = col - window_size / 2; j <= col + window_size / 2; j++)
				{
					if (i >= 0 && i < height &&
						j >= 0 && j < width)
					{
						uchar *old_data = old_img.ptr<uchar>(i);
						int wind_i = i - row + window_size / 2;
						int wind_j = j - col + window_size / 2;
						double t = window[wind_i * window_size + wind_j];
						b += old_data[j * 3] * t;
						g += old_data[j * 3 + 1] * t;
						r += old_data[j * 3 + 2] * t;
						w += t;
					}
				}
				data[col * 3] = b / w;
				data[col * 3 + 1] = g / w;
				data[col * 3 + 2] = r / w;
			}
		}
	}
	else
	{
		for (int row = 0; row < height; row++)
		{
			uchar *data = pDoc->image.ptr<uchar>(row);
			for (int col = 0; col < width; col++)
			{
				double r = 0, g = 0, b = 0;
				double w = 0;
				for (int i = row - window_size / 2; i <= row + window_size / 2; i++)
				for (int j = col - window_size / 2; j <= col + window_size / 2; j++)
				{
					if (i >= 0 && i < height &&
						j >= 0 && j < width)
					{
						uchar *old_data = old_img.ptr<uchar>(i);
						int wind_i = i - row + window_size / 2;
						int wind_j = j - col + window_size / 2;
						double t = window[wind_i * window_size + wind_j];
						b += old_data[j] * t;
						w += t;
					}
				}
				data[col] = b / w;
			}
		}
	}
	
	delete[] window;
	pView->Invalidate();
}

inline double gaussian_function(double x, double y, double delta)
{
	double tds = 0.1 + delta;
	double pi = 3.14159265359;
	return exp(-(x*x + y*y) / tds) / (tds*pi);
}