
// LogicAnalyser.h : main header file for the LogicAnalyser application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CLogicAnalyserApp:
// See LogicAnalyser.cpp for the implementation of this class
//

class CLogicAnalyserApp : public CWinApp
{
public:
	CLogicAnalyserApp() noexcept;


// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// Implementation

public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CLogicAnalyserApp theApp;
