#include "pch.h"
#include "ScrollHelper.h"


static void GetClientRectSB(CWnd* pWnd, CRect& rect)
{
	ASSERT(pWnd != NULL);

	CRect winRect;
	pWnd->GetWindowRect(&winRect);
	pWnd->ScreenToClient(&winRect);

	pWnd->GetClientRect(&rect);

	int cxSB = ::GetSystemMetrics(SM_CXVSCROLL);
	int cySB = ::GetSystemMetrics(SM_CYHSCROLL);

	if (winRect.right >= (rect.right + cxSB))
		rect.right += cxSB;
	if (winRect.bottom >= (rect.bottom + cySB))
		rect.bottom += cySB;
}

CScrollHelper::CScrollHelper()
{
	m_attachWnd = NULL;
	m_pageSize = CSize(0, 0);
	m_displaySize = CSize(0, 0);
	m_scrollPos = CSize(0, 0);
}

CScrollHelper::~CScrollHelper()
{
	DetachWnd();
}

void CScrollHelper::AttachWnd(CWnd* pWnd)
{
	m_attachWnd = pWnd;
}

void CScrollHelper::DetachWnd()
{
	m_attachWnd = NULL;
}

void CScrollHelper::SetDisplaySize(int displayWidth, int displayHeight)
{
	m_displaySize = CSize(displayWidth, displayHeight);

	if (m_attachWnd != NULL && ::IsWindow(m_attachWnd->m_hWnd))
		UpdateScrollInfo();
}

const CSize& CScrollHelper::GetDisplaySize() const
{
	return m_displaySize;
}

const CSize& CScrollHelper::GetScrollPos() const
{
	return m_scrollPos;
}

const CSize& CScrollHelper::GetPageSize() const
{
	return m_pageSize;
}

void CScrollHelper::ScrollToOrigin(bool scrollLeft, bool scrollTop)
{
	if (m_attachWnd == NULL)
		return;

	if (scrollLeft)
	{
		if (m_displaySize.cx > 0 && m_pageSize.cx > 0 && m_scrollPos.cx > 0)
		{
			int deltaPos = -m_scrollPos.cx;
			m_scrollPos.cx += deltaPos;
			m_attachWnd->SetScrollPos(SB_HORZ, m_scrollPos.cx, TRUE);
			m_attachWnd->ScrollWindow(-deltaPos, 0);
		}
	}

	if (scrollTop)
	{
		if (m_displaySize.cy > 0 && m_pageSize.cy > 0 && m_scrollPos.cy > 0)
		{
			int deltaPos = -m_scrollPos.cy;
			m_scrollPos.cy += deltaPos;
			m_attachWnd->SetScrollPos(SB_VERT, m_scrollPos.cy, TRUE);
			m_attachWnd->ScrollWindow(0, -deltaPos);
		}
	}
}

void CScrollHelper::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_attachWnd == NULL)
		return;

	const int lineOffset = 60;

	int deltaPos = 0;
	switch (nSBCode)
	{
	case SB_LINELEFT:
		deltaPos = -lineOffset;
		break;

	case SB_LINERIGHT:
		deltaPos = lineOffset;
		break;

	case SB_PAGELEFT:
		deltaPos = -m_pageSize.cx;
		break;

	case SB_PAGERIGHT:
		deltaPos = m_pageSize.cx;
		break;

	case SB_THUMBTRACK:
		deltaPos = Get32BitScrollPos(SB_HORZ, pScrollBar) - m_scrollPos.cx;
		break;

	case SB_THUMBPOSITION:
		deltaPos = Get32BitScrollPos(SB_HORZ, pScrollBar) - m_scrollPos.cx;
		break;

	default:
		return;
	}

	int newScrollPos = m_scrollPos.cx + deltaPos;

	if (newScrollPos < 0)
		deltaPos = -m_scrollPos.cx;

	int maxScrollPos = m_displaySize.cx - m_pageSize.cx;
	if (newScrollPos > maxScrollPos)
		deltaPos = maxScrollPos - m_scrollPos.cx;

	if (deltaPos != 0)
	{
		m_scrollPos.cx += deltaPos;
		m_attachWnd->SetScrollPos(SB_HORZ, m_scrollPos.cx, TRUE);
		m_attachWnd->ScrollWindow(-deltaPos, 0);
	}
}

void CScrollHelper::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (m_attachWnd == NULL)
		return;

	const int lineOffset = 60;

	int deltaPos = 0;
	switch (nSBCode)
	{
	case SB_LINEUP:
		deltaPos = -lineOffset;
		break;

	case SB_LINEDOWN:
		deltaPos = lineOffset;
		break;

	case SB_PAGEUP:
		deltaPos = -m_pageSize.cy;
		break;

	case SB_PAGEDOWN:
		deltaPos = m_pageSize.cy;
		break;

	case SB_THUMBTRACK:
		deltaPos = Get32BitScrollPos(SB_VERT, pScrollBar) - m_scrollPos.cy;
		break;

	case SB_THUMBPOSITION:
		deltaPos = Get32BitScrollPos(SB_VERT, pScrollBar) - m_scrollPos.cy;
		break;

	default:
		return;
	}

	int newScrollPos = m_scrollPos.cy + deltaPos;

	if (newScrollPos < 0)
		deltaPos = -m_scrollPos.cy;

	int maxScrollPos = m_displaySize.cy - m_pageSize.cy;
	if (newScrollPos > maxScrollPos)
		deltaPos = maxScrollPos - m_scrollPos.cy;

	if (deltaPos != 0)
	{
		m_scrollPos.cy += deltaPos;
		m_attachWnd->SetScrollPos(SB_VERT, m_scrollPos.cy, TRUE);
		m_attachWnd->ScrollWindow(0, -deltaPos);
	}
}

BOOL CScrollHelper::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	if (m_attachWnd == NULL)
		return FALSE;

	int scrollMin = 0, scrollMax = 0;
	m_attachWnd->GetScrollRange(SB_VERT, &scrollMin, &scrollMax);
	if (scrollMin == scrollMax)
		return FALSE;

	int numScrollIncrements = abs(zDelta) / WHEEL_DELTA;

	int numScrollLinesPerIncrement = 0;
	::SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &numScrollLinesPerIncrement, 0);

	if (numScrollLinesPerIncrement == WHEEL_PAGESCROLL)
	{
		OnVScroll(zDelta > 0 ? SB_PAGEUP : SB_PAGEDOWN, 0, NULL);
		return TRUE;
	}

	int numScrollLines = numScrollIncrements * numScrollLinesPerIncrement;

	numScrollLines = max(numScrollLines / 3, 1);

	for (int i = 0; i < numScrollLines; ++i)
	{
		OnVScroll(zDelta > 0 ? SB_LINEUP : SB_LINEDOWN, 0, NULL);
	}

	return TRUE;
}

void CScrollHelper::OnSize(UINT nType, int cx, int cy)
{
	UpdateScrollInfo();
}

int CScrollHelper::Get32BitScrollPos(int bar, CScrollBar* pScrollBar)
{
	ASSERT(m_attachWnd != NULL);
	HWND hWndScroll;
	if (pScrollBar == NULL)
		hWndScroll = m_attachWnd->m_hWnd;
	else
		hWndScroll = pScrollBar->m_hWnd;

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_TRACKPOS;
	::GetScrollInfo(hWndScroll, bar, &si);

	int scrollPos = si.nTrackPos;

	return scrollPos;
}

void CScrollHelper::UpdateScrollInfo()
{
	if (m_attachWnd == NULL)
		return;

	CRect rect;
	GetClientRectSB(m_attachWnd, rect);
	CSize windowSize(rect.Width(), rect.Height());

	CSize deltaPos(0, 0);
	UpdateScrollBar(SB_HORZ, windowSize.cx, m_displaySize.cx,
		m_pageSize.cx, m_scrollPos.cx, deltaPos.cx);

	UpdateScrollBar(SB_VERT, windowSize.cy, m_displaySize.cy,
		m_pageSize.cy, m_scrollPos.cy, deltaPos.cy);

	if (deltaPos.cx != 0 || deltaPos.cy != 0)
	{
		m_attachWnd->ScrollWindow(deltaPos.cx, deltaPos.cy);
	}
}

void CScrollHelper::UpdateScrollBar(int bar, int windowSize, int displaySize,
	LONG& pageSize, LONG& scrollPos, LONG& deltaPos)
{
	int scrollMax = 0;
	deltaPos = 0;
	if (windowSize < displaySize)
	{
		scrollMax = displaySize - 1;
		if (pageSize > 0 && scrollPos > 0)
		{
			scrollPos = (LONG)(1.0 * scrollPos * windowSize / pageSize);
		}
		pageSize = windowSize;
		scrollPos = min(scrollPos, displaySize - pageSize - 1);
		deltaPos = m_attachWnd->GetScrollPos(bar) - scrollPos;
	}
	else
	{
		pageSize = 0;
		scrollPos = 0;
		deltaPos = m_attachWnd->GetScrollPos(bar);
	}

	SCROLLINFO si;
	memset(&si, 0, sizeof(SCROLLINFO));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	si.nMin = 0;
	si.nMax = scrollMax;
	si.nPage = pageSize;
	si.nPos = scrollPos;
	m_attachWnd->SetScrollInfo(bar, &si, TRUE);
}
