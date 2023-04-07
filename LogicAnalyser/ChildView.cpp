
// ChildView.cpp : implementation of the CChildView class
//

#include "pch.h"
#include "framework.h"
#include "LogicAnalyser.h"
#include "ChildView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	// Since all of the logic is in this view, we parse the command line arguments here
	LPCWSTR commandLine = GetCommandLineW();
	int argc = 0;
	LPWSTR *argv = CommandLineToArgvW(commandLine, &argc);

	for (int i = 1 ; i < argc ; i++)
	{
		CStringA theArg(argv[i]);
		if (theArg.GetAt(0) == '-')
		{
			switch (theArg.GetAt(1))
			{
			case 'f':
				i++;
				if (i < argc)
				{
					CStringA theParam(argv[i]);
					mFillPercent = atof(theParam);
					continue;
				}
				break;
			case 'n':
				i++;
				if (i < argc)
				{
					CStringA theParam(argv[i]);
					mNumEntries = atoi(theParam);
					continue;
				}
				break;
			case 'z':
				i++;
				if (i < argc)
				{
					CStringA theParam(argv[i]);
					mZoomLevel = dsimtime(atof(theParam));
					continue;
				}
				break;
			default:
				break;
			}

		}
	}

	// Init the rest of the data
	mCaptureData = new CaptureData[mNumEntries];
	mFillNumEntries = (int) (mNumEntries * mFillPercent);
	mAfterTriggerRemainingNumEntries = mNumEntries - mFillNumEntries;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_SIZE()
	ON_COMMAND(ID_ZOOMIN, &CChildView::OnZoomin)
	ON_COMMAND(ID_ZOOMOUT, &CChildView::OnZoomout)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), nullptr);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);

	dc.FillSolidRect(0, 0, rect.right, rect.bottom, RGB(0, 0, 0));

	CSize scrollPos = mScroller.GetScrollPos();
	CSize scrollSize = mScroller.GetPageSize();
	double viewRatioFromLeft = double(scrollPos.cx) / double(scrollSize.cx);

	ABSTIME viewTimeStart = 0;
	if (scrollSize.cx > 0)
	{
		viewTimeStart = (mZoomLevel * scrollPos.cx) / (scrollSize.cx * kSubDivisions);
	}
	viewTimeStart += mBufferTimeMin;
	ABSTIME viewTimeEnd = mBufferTimeMax;

	int actualIndex = 0;
	if (mCaptureLooped)
	{
		actualIndex = mCaptureIndex;
	}

	int ypos = 0;
	// TODO: Better search instead of linear
	for (int i = 0 ; i < mNumCapturedCapped ; i++)
	{
		CaptureData &data = mCaptureData[actualIndex];

		ypos += 16;
		actualIndex++;
		if (actualIndex >= mNumEntries)
		{
			actualIndex = 0;
		}

		if (data.mTime >= viewTimeEnd)
		{
			break;
		}

		ABSTIME deltaTime = data.mTime - viewTimeStart;
		if (deltaTime < 0)
		{
			continue;
		}

		int pixelPosition = (int)((rect.right * deltaTime) / mZoomLevel);
		dc.SetTextColor(RGB(255, 255, 255));
		CString text;
		text.Format(L"Hello %08x" , data.mData);
		dc.TextOut(pixelPosition , ypos, text);
	}
}



void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	mScroller.OnHScroll(nSBCode, nPos, pScrollBar);
//	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
	RedrawWindow();
}


void CChildView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	mScroller.OnSize(nType, cx, cy);
}


BOOL CChildView::Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext)
{
	mScroller.AttachWnd(this);

	return CWnd::Create(lpszClassName, lpszWindowName, dwStyle, rect, pParentWnd, nID, pContext);
}


void CChildView::OnZoomin()
{
	mZoomLevel /= 2;
	UpdateScrollbars();
	RedrawWindow();
}


void CChildView::OnZoomout()
{
	mZoomLevel *= 2;
	UpdateScrollbars();
	RedrawWindow();
}


BOOL CChildView::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

//	return CWnd::OnEraseBkgnd(pDC);
	// This stops the window clearing itself. Removes bright background flickering.
	return TRUE;
}


void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	// Pretend we get data on a timer

	CWnd::OnTimer(nIDEvent);

	static ABSTIME faketime = 0;
	static unsigned int data = 0;

	CaptureData fakeCapture;
	fakeCapture.mTime = faketime;
	fakeCapture.mData = data;

	bool needRefresh = AddCaptureDataEvent(fakeCapture);

	if (needRefresh)
	{
		UpdateScrollbars();
		RedrawWindow();
	}

	faketime += dsimtime(1.0f);
	data += 1;
	data = data | (data << 8) | (data << 16) | (data << 24);
}

void CChildView::UpdateScrollbars(void)
{
	CRect rect;
	GetClientRect(&rect);
	mScroller.SetDisplaySize((int)(((double)(kSubDivisions * rect.right)) * (realtime(mBufferTime)/ realtime(mZoomLevel))), 512);
}

// Return: true indicates a visual refresh is needed
bool CChildView::AddCaptureDataEvent(CaptureData &data)
{
	// Decide if we want to store the capture data after trigger
	if (mTriggered && mCaptureLooped && mAfterTriggerRemainingNumEntries <= 0)
	{
		return false;
	}
	// First entry sets the min time
	if (!mCaptureLooped && mCaptureIndex == 0)
	{
		mBufferTimeMin = data.mTime;
	}

	CaptureData justRetired;
	if (mCaptureLooped)
	{
		justRetired = mCaptureData[mCaptureIndex];
		mBufferTimeMin = justRetired.mTime;
	}
	mCaptureData[mCaptureIndex] = data;
	mCaptureIndex++;
	if (!mCaptureLooped)
	{
		mNumCapturedCapped++;
	}
	if (mCaptureIndex >= mNumEntries)
	{
		mCaptureIndex = 0;
		mCaptureLooped = true;
	}

	mBufferTimeMax = data.mTime;
	mBufferTime = mBufferTimeMax - mBufferTimeMin;

	if (mTriggered && mCaptureLooped && (mAfterTriggerRemainingNumEntries > 0))
	{
		mAfterTriggerRemainingNumEntries--;
	}

	return true;
}
