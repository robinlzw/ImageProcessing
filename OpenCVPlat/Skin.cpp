// Skin.cpp : 实现文件
//

#include "stdafx.h"
#include "OpenCVPlat.h"
#include "Skin.h"
#include "afxdialogex.h"
#include "MainFrm.h"
#include "OpenCVPlatDoc.h"
#include "OpenCVPlatView.h"

// CSkin 对话框

IMPLEMENT_DYNAMIC(CSkin, CDialogEx)

CSkin::CSkin(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSkin::IDD, pParent)
	, face_info(_T(""))
{
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	old_img = pDoc->image.clone();
	rub_img = old_img.clone();
}

CSkin::~CSkin()
{
}

void CSkin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INFO, face_info);
	DDV_MaxChars(pDX, face_info, 100);
}


BEGIN_MESSAGE_MAP(CSkin, CDialogEx)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()


// CSkin 消息处理程序


BOOL CSkin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_RUB_SKIN);
	pSlidCtrl->SetRange(0, 100);
	pSlidCtrl->SetPos(0);
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_WHITE_SKIN);
	pSlidCtrl->SetRange(0, 100);
	pSlidCtrl->SetPos(0);

	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	COpenCVPlatView *pView = (COpenCVPlatView*)pMain->GetActiveView();

	face_detect(pDoc->image);

	if (faces.size() == 0)
	{
		face_info.SetString(_T("没有检测到人脸！"));
	}
	else
	{
		face_info.Format(_T("检测到了%d张人脸！"), faces.size());
	}
	UpdateData(false);

	int window_size = 11;
	int height = pDoc->image.size().height;
	int width = pDoc->image.size().width;

	for (int i = 0; i < faces.size(); i++)
	{
		int sx = faces[i].x;
		int sy = faces[i].y;
		int ex = faces[i].x + faces[i].width;
		int ey = faces[i].y + faces[i].height;
		if (pDoc->image.channels() == 3)
		{
			for (int row = sy; row < ey; row++)
			{
				uchar *data = rub_img.ptr<uchar>(row);
				for (int col = sx; col < ex; col++)
				{
					double r = 0, g = 0, b = 0;
					double wr = 0, wg = 0, wb = 0;
					for (int i = row - window_size / 2; i <= row + window_size / 2; i++)
					for (int j = col - window_size / 2; j <= col + window_size / 2; j++)
					{
						if (i >= 0 && i < height &&
							j >= 0 && j < width)
						{
							uchar *old_data = old_img.ptr<uchar>(i);
							int wind_i = i - row + window_size / 2;
							int wind_j = j - col + window_size / 2;
							double tb = wij(i, j, row, col, old_data[j * 3], data[col * 3]);
							double tg = wij(i, j, row, col, old_data[j * 3 + 1], data[col * 3 + 1]);
							double tr = wij(i, j, row, col, old_data[j * 3 + 2], data[col * 3 + 2]);
							b += old_data[j * 3] * tb;
							g += old_data[j * 3 + 1] * tg;
							r += old_data[j * 3 + 2] * tr;
							wb += tb;
							wg += tg;
							wr += tr;
						}
					}
					data[col * 3] = b / wb;
					data[col * 3 + 1] = g / wg;
					data[col * 3 + 2] = r / wr;
				}
			}
		}
		else
		{
			for (int row = sy; row < ey; row++)
			{
				uchar *data = rub_img.ptr<uchar>(row);
				for (int col = sx; col < ex; col++)
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
							double t = wij(i, j, row, col, old_data[j], data[col]);
							b += old_data[j] * t;
							w += t;
						}
					}
					data[col] = b / w;
				}
			}
		}
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// 异常:  OCX 属性页应返回 FALSE
}


void CSkin::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	CSliderCtrl *pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_RUB_SKIN);
	double rub_value = pSlidCtrl->GetPos() / 100.0;
	pSlidCtrl = (CSliderCtrl*)GetDlgItem(IDC_WHITE_SKIN);
	double white_value = pSlidCtrl->GetPos() / 100.0;
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
	CMainFrame* pMain = (CMainFrame*)AfxGetApp()->m_pMainWnd;
	COpenCVPlatDoc *pDoc = (COpenCVPlatDoc*)pMain->GetActiveDocument();
	COpenCVPlatView *pView = (COpenCVPlatView*)pMain->GetActiveView();
	int height = pDoc->image.size().height;
	int width = pDoc->image.size().width;

	
	for (int i = 0; i < faces.size(); i++)
	{
		int sx = faces[i].x;
		int sy = faces[i].y;
		int ex = faces[i].x + faces[i].width;
		int ey = faces[i].y + faces[i].height;

		int size = faces[i].width * faces[i].height;

		for (int row = sy; row < ey; row++)
		{
			uchar *old_data = old_img.ptr<uchar>(row);
			uchar *data = pDoc->image.ptr<uchar>(row);
			uchar *rub_data = rub_img.ptr<uchar>(row);

			for (int col = sx; col < ex; col++)
			{
				double ratio = mask(col, row, (sx + ex) / 2, (sy + ey) / 2,
					ex - sx, ey - sy);
				double rv = rub_value * ratio;

				if (pDoc->image.channels() == 3)
				{
					data[col * 3] = (1 - rv) * old_data[col * 3]
						+ rv * rub_data[col * 3];
					data[col * 3 + 1] = (1 - rv) * old_data[col * 3 + 1]
						+ rv * rub_data[col * 3 + 1];
					data[col * 3 + 2] = (1 - rv) * old_data[col * 3 + 2]
						+ rv * rub_data[col * 3 + 2];
				}
				else
				{
					data[col] = (1 - rv) * old_data[col]
						+ rv * rub_data[col];
				}
			}
		}
		cv::Mat hsv_white_img = pDoc->image.clone();
		if (pDoc->image.channels() == 3)
			cv::cvtColor(pDoc->image, hsv_white_img, CV_BGR2HSV);
		for (int row = sy; row < ey; row++)
		{
			uchar *data = hsv_white_img.ptr<uchar>(row);
			for (int col = sx; col < ex; col++)
			{
				double ratio = mask(col, row, (sx + ex) / 2, (sy + ey) / 2,
					ex - sx, ey - sy);
				double wv = white_value * ratio;
				double value = 1 - 0.1 * wv;
				double li = 1 + 0.1 * wv;

				if (pDoc->image.channels() == 3)
				{
					data[col * 3 + 1] = data[col * 3 + 1] * value;
					double t = data[col * 3 + 2] * li;
					data[col * 3 + 2] = t > 255 ? 255 : t;
				}
				else
				{
					double t = data[col] * li;
					data[col] = t > 255 ? 255 : t;
				}
			}
		}
		if (pDoc->image.channels() == 3)
			cv::cvtColor(hsv_white_img, pDoc->image, CV_HSV2BGR);
	}
	
	pView->Invalidate();
}

double wij(int i, int j, int x, int y, uchar gij, uchar gxy)
{
	double tds = 400;
	double tdr = 500;
	int ix = (i - x)*(i - x);
	int jy = (j - y)*(j - y);
	int gg = (gij - gxy)*(gij - gxy);
	double w = exp(-(ix + jy)/tds - gg/tdr);
	return w;
}

void CSkin::face_detect(const cv::Mat &frame)
{
	char *face_cascade_name = "haarcascade_frontalface_alt.xml";
	cv::CascadeClassifier face_cascade;
	if (!face_cascade.load(face_cascade_name))
	{
		return;
	}
	if (frame.cols > 0 && frame.rows > 0)
	{
		cv::Mat frame_gray;
		if (frame.channels() == 3)
			cvtColor(frame, frame_gray, CV_BGR2GRAY);
		else
			frame_gray = frame.clone();
		equalizeHist(frame_gray, frame_gray);
		face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CV_HAAR_SCALE_IMAGE, cv::Size(30, 30));
	}
}

inline double mask(int x, int y, int cx, int cy, int w, int h)
{
	double xt = x - cx;
	double yt = y - cy;
	yt = yt * w / (double)h;
	double r = (xt*xt + yt*yt) / (w*w/4);
	if (r <= 0.7)
		return 1;
	else if (r <= 1)
		return 1 - 10 * (r - 0.7)/3;
	else
		return 0;
}