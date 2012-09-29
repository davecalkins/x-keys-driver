
// XKeysDriverDlg.cpp : implementation file
//

#include "stdafx.h"
#include "XKeysDriver.h"
#include "XKeysDriverDlg.h"
#include "Utils.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// if enabled, write the log file
//#define ENABLE_LOGFILE

// if enabled, reload the config file if its changed on every event
// if not, only load once at startup, assuming it wont change
// enable this while testing and modifying the config file a lot.
// once the config file stabilizes, disable it to save checking the
// file mod time on every event.
//#define ENABLE_CONFIG_FILE_LIVE_RELOAD

#define TRAY_ICON_ID                       101
#define TRAY_ICON_UPDATE_TIMER_ID          102
#define TRAY_ICON_UPDATE_TIMER_INTERVAL_MS 5000

// unique ID for PIE devices
#define PIE_VID 0x5F3

// received when the device has been disconnected
#define PIE_DEVICE_DISCONNECT_ERROR 307

// indicates a general incoming data message
#define PIE_GENERAL_INCOMING_DATA   0x7a

// keyboard geometry, total number of columns and rows
#define KB_NUM_COLS 12
#define KB_NUM_ROWS 7

// config file to load
#define CONFIG_FILE _T("XKeysDriverConfig.xml")

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CXKeysDriverDlg dialog




CXKeysDriverDlg::CXKeysDriverDlg(CWnd* pParent /*=NULL*/)
: CDialog(CXKeysDriverDlg::IDD, pParent)
, cfg(*this)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

   m_hDevice = -1;
}

void CXKeysDriverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CXKeysDriverDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
   ON_MESSAGE(WM_KILL_APP,OnKillApp)
   ON_MESSAGE(WM_TRAYICON_NOTIFY,OnTrayIconNotify)
   ON_WM_DESTROY()
   ON_COMMAND(ID_EXIT, &CXKeysDriverDlg::OnExit)
   ON_WM_TIMER()
   ON_MESSAGE(WM_PIE_FINDANDSTART,OnPIEFindAndStart)
END_MESSAGE_MAP()


// CXKeysDriverDlg message handlers

BOOL CXKeysDriverDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

   Log(_T("STARTING"));

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

   // get keyboard layout
   m_hKeyboardLayout = GetKeyboardLayout(0);

	// setup config file path
   configFilePath.Format(_T("%s%s"), theApp.appDir, CONFIG_FILE);

   // initial load of the config file
   cfg.LoadConfigFile(TRUE);

   // load tray icons and set the initial icon
   m_trayPopupMenu.LoadMenu(IDR_TRAYPOPUP);
   m_hIconUnknown = theApp.LoadIcon(IDR_UNKNOWN);
   m_hIconOffline = theApp.LoadIcon(IDR_OFFLINE);
   m_hIconOnline = theApp.LoadIcon(IDR_ONLINE);
   m_hTrayIcon = m_hIconUnknown;
   UpdateTrayIcon();

   // setup timer to periodically update the tray icon, this timer also
   // triggers a reconnect to the device if necessary
   SetTimer(TRAY_ICON_UPDATE_TIMER_ID,TRAY_ICON_UPDATE_TIMER_INTERVAL_MS,NULL);

   // initial connection attempt
   PostMessage(WM_PIE_FINDANDSTART);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXKeysDriverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CXKeysDriverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CXKeysDriverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CXKeysDriverDlg::OnKillApp(WPARAM wParam, LPARAM lParam)
{
   PostThreadMessage(AfxGetThread()->m_nThreadID,WM_QUIT,0,0);
   return 0;
}

void CXKeysDriverDlg::UpdateTrayIcon()
{
   // ensure the correct tray icon is displayed
   NOTIFYICONDATA nid;
   ZeroMemory(&nid,sizeof(NOTIFYICONDATA));
   nid.cbSize = sizeof(NOTIFYICONDATA);
   nid.hWnd = GetSafeHwnd();
   nid.uID = TRAY_ICON_ID;
   nid.uFlags = NIF_MESSAGE | NIF_TIP;
   if (m_hTrayIcon != NULL)
   {
      nid.uFlags |= NIF_ICON;
      nid.hIcon = m_hTrayIcon;
   }
   nid.uCallbackMessage = WM_TRAYICON_NOTIFY;
   _tcscpy_s(nid.szTip,_T("XKeysDriver"));

   if (m_hTrayIcon != NULL)
   {
      Shell_NotifyIcon(NIM_ADD,&nid);
      Shell_NotifyIcon(NIM_MODIFY,&nid);
   }
   else
      Shell_NotifyIcon(NIM_DELETE,&nid);
}

LRESULT CXKeysDriverDlg::OnTrayIconNotify(WPARAM wParam, LPARAM lParam)
{
   // show popup menu when user right-clicks on the tray icon
   if ((lParam == WM_RBUTTONDOWN) ||
       (lParam == WM_CONTEXTMENU))
   {
      POINT p;
      GetCursorPos(&p);

      SetForegroundWindow();
      CMenu* pSubMenu = m_trayPopupMenu.GetSubMenu(0);
      if (pSubMenu != NULL)
         pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN,p.x,p.y,this);
   }

   return 0;
}

void CXKeysDriverDlg::OnDestroy()
{
   CDialog::OnDestroy();

   m_hTrayIcon = NULL;
   UpdateTrayIcon();

   if (m_hDevice != -1)
   {
      Log(_T("closing interface with device"));
      CleanupInterface(m_hDevice);
      CloseInterface(m_hDevice);
      m_hDevice = -1;
   }

   Log(_T("EXITING"));
}

void CXKeysDriverDlg::OnExit()
{
   KillTimer(TRAY_ICON_UPDATE_TIMER_ID);

   PostMessage(WM_KILL_APP);
}

void CXKeysDriverDlg::OnTimer(UINT_PTR nIDEvent)
{
   if (nIDEvent == TRAY_ICON_UPDATE_TIMER_ID)
   {
      UpdateTrayIcon();

      // if offline, attempt to connect to the device
      if (m_hTrayIcon == m_hIconOffline)
         PostMessage(WM_PIE_FINDANDSTART);
   }
   else
      CDialog::OnTimer(nIDEvent);
}

void CXKeysDriverDlg::Log(LPCTSTR fmtString, ...)
{
#ifndef ENABLE_LOGFILE
   return;
#endif

   va_list vl;
   va_start(vl,fmtString);

   CString msg;
   msg.FormatV(fmtString,vl);

   CTime now = CTime::GetCurrentTime();
   
   CString outText;
   outText.Format(_T("%s - %s\n"),
      now.Format(_T("%Y%m%d @ %H%M%S")),
      msg);

   TRACE1("%s", outText);

   CString logFilePath;
   logFilePath.Format(_T("%slog.txt"), theApp.appDir);

   CStdioFile f;
   if (!f.Open(logFilePath,
      CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::typeText))
      return;

   f.SeekToEnd();
   f.WriteString(outText);
   f.Flush();
   f.Close();
}

LRESULT CXKeysDriverDlg::OnPIEFindAndStart(WPARAM wParam, LPARAM lParam)
{
   DWORD result;
   long deviceData[MAX_XKEY_DEVICES * 4];
   long count;
   int pid;
   long usage;
   long hnd;

   // enumerate available devices
   result = EnumeratePIE(PIE_VID,deviceData,count);
   if (result != 0)
   {
      Log(_T("ERROR: Unable to enumerate available devices (%d)"), result);
      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return 0;
   }
   Log(_T("enumerated %d available devices"), count);

   // are there any devices present?
   if (count == 0)
   {
      Log(_T("ERROR: No devices present"));
      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return 0;
   }

   // look for the first device of the expected type
   BOOL foundDevice = FALSE;
   for (long i = 0; (i < count) && !foundDevice; i++)
   {
      pid = deviceData[i*4 + 0];
      usage = deviceData[i*4 + 2];
      hnd = deviceData[i*4 + 3];

      Log(_T("device%d, pid == 0x%x, usage == 0x%x, hnd == 0x%x"), i, pid, usage, hnd);

      if ((usage == 0xC && pid != 37) && (pid == 0x308 || pid == 0x309 || pid==0x30A))
      {
         foundDevice = TRUE;
         Log(_T("found matching device%d"), i, hnd);
      }
   }

   // no suitable device found
   if (!foundDevice)
   {
      Log(_T("ERROR: No suitable devices present"));
      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return 0;
   }

   // setup interface with the chosen device
   result = SetupInterface(hnd);
   if (result != 0)
   {
      Log(_T("ERROR: Unable to setup communications with the device (%d)"), result);
      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return 0;
   }

   // set callback for data events
   result = SetDataCallback(hnd,piDataChange,_PIEHandleDataEvent);
   if (result != 0)
   {
      Log(_T("ERROR: Unable to set callback for data events (%d)"), result);

      CleanupInterface(hnd);
      CloseInterface(hnd);

      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return 0;
   }

   // SUCCESS!
   m_hDevice = hnd;
   Log(_T("successfully setup communications with the device"));
   m_hTrayIcon = m_hIconOnline;
   UpdateTrayIcon();

   return 0;
}

DWORD __stdcall CXKeysDriverDlg::_PIEHandleDataEvent(UCHAR *pData, DWORD deviceID, DWORD error)
{
   return theApp.pDlg->OnPIEHandleDataEvent(pData,deviceID,error);
}

DWORD CXKeysDriverDlg::OnPIEHandleDataEvent(UCHAR *pData, DWORD deviceID, DWORD error)
{
   // device disconnected!
   if (error == PIE_DEVICE_DISCONNECT_ERROR)
   {
      Log(_T("ERROR: device disconnected (%d)"), PIE_DEVICE_DISCONNECT_ERROR);
      CleanupInterface(m_hDevice);
      CloseInterface(m_hDevice);
      m_hDevice = -1;
      m_hTrayIcon = m_hIconOffline;
      UpdateTrayIcon();
      return TRUE;
   }

   // log received data
   /*
   Log(_T("from device: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x"),
      pData[0], pData[1], pData[2], pData[3], pData[4], pData[5], pData[6], pData[7], pData[8], pData[9],
      pData[10], pData[11], pData[12], pData[13], pData[14], pData[15], pData[16], pData[17], pData[18], pData[19],
      pData[20], pData[21], pData[22], pData[23], pData[24], pData[25], pData[26], pData[27], pData[28], pData[29],
      pData[30], pData[31]);
   */

   // ensure config file is loaded
#ifdef ENABLE_CONFIG_FILE_LIVE_RELOAD
   cfg.LoadConfigFile();
#endif

   // general incoming data
   if (pData[2] == PIE_GENERAL_INCOMING_DATA)
   {
      KBState s;

      // get pressed keys
      CString pk;
      for (int col = 0; col < KB_NUM_COLS; col++)
      {
         UCHAR colData = pData[3+col];
         for (int row = 0; row < KB_NUM_ROWS; row++)
         {
            if ((colData & (1 << row)) != 0)
            {
               s.pressedKeys[make_pair(col,row)] = true;
               CString q;
               q.Format(_T("%d,%d"), col, row);
               pk += q;
               pk += " ";
            }
         }
      }
      Log(_T("pressed keys: %s"), pk);

      // get key position
      UCHAR keyByte = pData[1];
      s.keyPos = 0;
      for (int bitPos = 0; bitPos < 8; bitPos++)
      {
         if ((keyByte & (1 << bitPos)) != 0)
            break;
         s.keyPos++;
         if (bitPos == 0)
            s.keyPos++;
         if (bitPos == 6)
            s.keyPos = 1;
      }
      Log(_T("key pos: %d"), s.keyPos);

      // pending data in card buffer?
      if (cardBuffer.size() > 0)
      {
         if ((cfg.cardReaderKeyPos == -1) ||
             (s.keyPos == cfg.cardReaderKeyPos))
         {
            CString q;
            for (vector<UCHAR>::iterator cbIter = cardBuffer.begin(); cbIter != cardBuffer.end(); ++cbIter)
               q += (TCHAR)*cbIter;
            Log(_T("card reader: %s"), q);

            DoActionSendInput(q);
         }

         cardBuffer.clear();
         return TRUE;
      }

      // trigger events
      cfg.TriggerEvents(s);
   }
   // barcode data, append to buffer
   else if ((pData[2] > 0) && (pData[2] <= 27))
   {
      int pos = 3;
      for (int i = 0; i < pData[2]; i++, pos++)
         cardBuffer.push_back(pData[pos]);
   }

   return TRUE;
}

void CXKeysDriverDlg::Config::LoadConfigFile(BOOL forceLoad /*= FALSE*/)
{
   CFileStatus fs;
   if (!CFile::GetStatus(outer.configFilePath,fs))
      return;

   if (!forceLoad)
   {
      if (fs.m_mtime <= configFileLoadTime)
         return;
   }

   configFileLoadTime = CTime::GetCurrentTime();

   outer.Log(_T("loading config file, \"%s\""), outer.configFilePath);

   int numEvents = 0;

   events.clear();

   TiXmlDocument doc(STRT2A(outer.configFilePath));
   if (!doc.LoadFile())
      return;

   TiXmlElement* pRootEle = doc.FirstChildElement("XKeysDriverConfig");
   if (pRootEle == NULL)
      return;

   TiXmlElement* pCardReaderEle = pRootEle->FirstChildElement("CardReader");
   if (pCardReaderEle != NULL)
   {
      const char* keyPositionStrA = pCardReaderEle->Attribute("keyPosition");
      if (keyPositionStrA != NULL)
         pCardReaderEle->Attribute("keyPosition",&cardReaderKeyPos);
      else
         cardReaderKeyPos = -1;
   }
   else
      cardReaderKeyPos = -1;

   TiXmlElement* pEventsEle = pRootEle->FirstChildElement("Events");
   if (pEventsEle == NULL)
      return;

   TiXmlElement* pEventEle = pEventsEle->FirstChildElement("Event");
   while (pEventEle != NULL)
   {
      KBEvent e;

      const char* keyPositionStrA = pEventEle->Attribute("keyPosition");
      if (keyPositionStrA != NULL)
         pEventEle->Attribute("keyPosition",&e.keyPosition);
      else
         e.keyPosition = -1;

      TiXmlElement* pKeysEle = pEventEle->FirstChildElement("Keys");
      if (pKeysEle == NULL)
         return;

      TiXmlElement* pKeyEle = pKeysEle->FirstChildElement("Key");
      while (pKeyEle != NULL)
      {
         KBKey k;

         pKeyEle->Attribute("col",&k.col);
         pKeyEle->Attribute("row",&k.row);

         e.keys.push_back(k);

         pKeyEle = pKeyEle->NextSiblingElement("Key");
      }

      TiXmlElement* pActionsEle = pEventEle->FirstChildElement("Actions");
      if (pActionsEle == NULL)
         return;

      TiXmlElement* pActionEle = pActionsEle->FirstChildElement("Action");
      while (pActionEle != NULL)
      {
         KBAction a;

         const char* actionTypeStrA = pActionEle->Attribute("type");
         if (actionTypeStrA == NULL)
            continue;
         CString actionTypeStr(STRA2T(actionTypeStrA));
         if (actionTypeStr.CompareNoCase(_T("SendInput")) == 0)
            a.actionType = KBAction::AT_SENDINPUT;
         else
            continue;

         const char* argStrA = pActionEle->Attribute("arg");
         if (argStrA == NULL)
            continue;
         CString argStr(STRA2T(argStrA));
         a.arg = argStr;

         e.actions.push_back(a);

         pActionEle = pActionEle->NextSiblingElement("Action");
      }

      events.push_back(e);
      numEvents++;

      pEventEle = pEventEle->NextSiblingElement("Event");
   }

   outer.Log(_T("loaded %d events"), numEvents);
}

void CXKeysDriverDlg::Config::TriggerEvents(KBState& s)
{
   std::vector<KBEvent> eventsToTrigger;

   for (vector<KBEvent>::iterator eventsIter = events.begin(); eventsIter != events.end(); ++eventsIter)
   {
      KBEvent& e = *eventsIter;

      if ((e.keyPosition != -1) && (s.keyPos != e.keyPosition))
         continue;

      BOOL triggerEvent = FALSE;
      for (std::vector<KBKey>::iterator keysIter = e.keys.begin(); (keysIter != e.keys.end()) && !triggerEvent; ++keysIter)
      {
         KBKey& k = *keysIter;
         if (s.pressedKeys.find(make_pair(k.col,k.row)) != s.pressedKeys.end())
            triggerEvent = TRUE;
      }

      if (triggerEvent)
         eventsToTrigger.push_back(e);
   }

   if ((eventsToTrigger.size() == 0) || (eventsToTrigger.size() > 1))
      return;

   ExecuteEvent(eventsToTrigger[0]);
}

void CXKeysDriverDlg::Config::ExecuteEvent(KBEvent& e)
{
   for (vector<KBAction>::iterator actionsIter = e.actions.begin(); actionsIter != e.actions.end(); ++actionsIter)
   {
      KBAction& a = *actionsIter;

      if (a.actionType == KBAction::AT_SENDINPUT)
      {
         outer.DoActionSendInput(a.arg);
      }
   }
}

void CXKeysDriverDlg::DoActionSendInput(LPCTSTR arg)
{
   int arglen = _tcslen(arg);
   for (int j = 0; j < arglen; j++)
   {
      _TCHAR c = arg[j];

      if ((c == _T('\\')) &&
          (j < (arglen-1)))
      {
         _TCHAR nc = arg[j+1];
         if (nc != _T('\\'))
         {
            if ((nc == _T('n')) || (nc == _T('N')))
               c = _T('\n');
            else if ((nc == _T('t')) || (nc == _T('T')))
               c = _T('\t');
         }
         j++;
      }
      
      INPUT inputArr[4];
      ZeroMemory(inputArr,sizeof(INPUT)*4);
      UINT numInputs = 0;

      SHORT vksr = VkKeyScanEx(c,m_hKeyboardLayout);
      BYTE vkCode = LOBYTE(vksr);
      BYTE shiftState = HIBYTE(vksr);
      BOOL shiftKeyPressed = (shiftState & 1) != 0;

      if (shiftKeyPressed)
      {
         inputArr[numInputs].type = INPUT_KEYBOARD;
         inputArr[numInputs].ki.wVk = VK_SHIFT;
         numInputs++;
      }

      inputArr[numInputs].type = INPUT_KEYBOARD;
      inputArr[numInputs].ki.wVk = vkCode;
      numInputs++;

      inputArr[numInputs].type = INPUT_KEYBOARD;
      inputArr[numInputs].ki.wVk = vkCode;
      inputArr[numInputs].ki.dwFlags = KEYEVENTF_KEYUP;
      numInputs++;

      if (shiftKeyPressed)
      {
         inputArr[numInputs].type = INPUT_KEYBOARD;
         inputArr[numInputs].ki.wVk = VK_SHIFT;
         inputArr[numInputs].ki.dwFlags = KEYEVENTF_KEYUP;
         numInputs++;
      }

      SendInput(numInputs,inputArr,sizeof(INPUT));
   }
}
