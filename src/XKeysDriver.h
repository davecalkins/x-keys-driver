
// XKeysDriver.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include "AppMsg.h"

class CXKeysDriverDlg;

// CXKeysDriverApp:
// See XKeysDriver.cpp for the implementation of this class
//

class CXKeysDriverApp : public CWinAppEx
{
public:
	CXKeysDriverApp();

// Overrides
	public:
	virtual BOOL InitInstance();

   CXKeysDriverDlg* pDlg;

   HANDLE hInstLock;

   CString appDir;

// Implementation

	DECLARE_MESSAGE_MAP()
   virtual int ExitInstance();
};

extern CXKeysDriverApp theApp;