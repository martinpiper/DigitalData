
// ChildView.h : interface of the CChildView class
//


#pragma once

#include "ScrollHelper.h"
#include "vsm.hpp"


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

//	CScrollBar mScrollBar;
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);

	CScrollHelper mScroller;
	afx_msg void OnZoomin();
	afx_msg void OnZoomout();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	ABSTIME mBufferTimeMin = 0;
	ABSTIME mBufferTimeMax = 0;
	ABSTIME mBufferTime = dsimtime(0.0f);
	RELTIME mZoomLevel = dsimtime(1.0f);

	int mNumEntries = 1000000;
	struct CaptureData
	{
		CaptureData() : mTime(0) , mData(0) , mNegativeEdge(0) , mPositiveEdge(0) , mTransitioning(0) , mUndefined(0)
		{
		}
		ABSTIME mTime;
		unsigned int mData;
		unsigned int mNegativeEdge;
		unsigned int mPositiveEdge;
		unsigned int mTransitioning;
		unsigned int mUndefined;
	};
	CaptureData *mCaptureData;
	int mCaptureIndex = 0;
	int mNumCapturedCapped = 0;
	double mFillPercent = 50.0f;
	int mFillNumEntries;
	bool mTriggered = false;
	int mTriggeredPos = 0;
	bool mCaptureLooped = false;
	int mAfterTriggerRemainingNumEntries = 0;

	bool AddCaptureDataEvent(CaptureData &data);

	const int kSubDivisions = 10;

private:
	void UpdateScrollbars(void);
};

