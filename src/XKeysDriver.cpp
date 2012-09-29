
// XKeysDriver.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "XKeysDriver.h"
#include "XKeysDriverDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CXKeysDriverApp

BEGIN_MESSAGE_MAP(CXKeysDriverApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CXKeysDriverApp construction

CXKeysDriverApp::CXKeysDriverApp()
: pDlg(NULL)
, hInstLock(NULL)
{
}


// The one and only CXKeysDriverApp object

CXKeysDriverApp theApp;


// CXKeysDriverApp initialization

BOOL CXKeysDriverApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("XKeysDriver"));

   TCHAR apbuf[MAX_PATH*2];
   GetModuleFileName(NULL,apbuf,MAX_PATH);
   appDir = apbuf;
   int bs = appDir.ReverseFind(_T('\\'));
   appDir.SetAt(bs+1,_T('\0'));

   CString cmdLine(m_lpCmdLine);
   cmdLine.MakeUpper();
   if (cmdLine.Find(_T("-SHUTDOWN")) >= 0)
   {
      HWND hWnd = FindWindow(NULL,_T("XKeysDriver-MainDlg-HIDDEN"));
      if (::IsWindow(hWnd))
         ::PostMessage(hWnd,WM_KILL_APP,0,0);

      return FALSE;
   }
   else
   {
      hInstLock = CreateMutex(NULL,FALSE,_T("XKeysDriver-InstanceLock-Mutex"));
      if (GetLastError() == ERROR_ALREADY_EXISTS)
         return FALSE;

      pDlg = new CXKeysDriverDlg;
      m_pMainWnd = pDlg;
      pDlg->Create(IDD_XKEYSDRIVER_DIALOG);

      return TRUE;
   }

}

int CXKeysDriverApp::ExitInstance()
{
   if (hInstLock != NULL)
   {
      CloseHandle(hInstLock);
      hInstLock = NULL;
   }

   if (pDlg != NULL)
   {
      pDlg->DestroyWindow();
      delete pDlg;
      pDlg = NULL;
   }

   return CWinAppEx::ExitInstance();
}
