
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
}



void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	mScroller.OnHScroll(nSBCode, nPos, pScrollBar);
//	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
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
	mZoomLevel /= 2.0f;
}


void CChildView::OnZoomout()
{
	mZoomLevel *= 2.0f;
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

	mMaxTime += 1.0f;

	mScroller.SetDisplaySize((int)(100.0f * mMaxTime * mZoomLevel), 0);
}
