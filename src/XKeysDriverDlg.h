
// XKeysDriverDlg.h : header file
//

#pragma once

#include <vector>
#include <map>
#include <utility>

// CXKeysDriverDlg dialog
class CXKeysDriverDlg : public CDialog
{
// Construction
public:
	CXKeysDriverDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_XKEYSDRIVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

   void UpdateTrayIcon();

   CMenu m_trayPopupMenu;

   HICON m_hIconUnknown;
   HICON m_hIconOffline;
   HICON m_hIconOnline;
   HICON m_hTrayIcon;

   long m_hDevice;

   HKL m_hKeyboardLayout;

   CString configFilePath;

   void Log(LPCTSTR fmtString, ...);

   std::vector<UCHAR> cardBuffer;

   void DoActionSendInput(LPCTSTR arg);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
   afx_msg LRESULT OnKillApp(WPARAM wParam, LPARAM lParam);
   afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam);
public:
   afx_msg void OnDestroy();
   afx_msg void OnExit();
   afx_msg void OnTimer(UINT_PTR nIDEvent);
   afx_msg LRESULT OnPIEFindAndStart(WPARAM wParam, LPARAM lParam);

   static DWORD __stdcall _PIEHandleDataEvent(UCHAR *pData, DWORD deviceID, DWORD error);
   DWORD OnPIEHandleDataEvent(UCHAR *pData, DWORD deviceID, DWORD error);

   class KBState
   {
   public:
      KBState() { keyPos = 0; }

      // column,row => true if pressed, false otherwise
      std::map<std::pair<int,int>,bool> pressedKeys;

      // 8 position key
      int keyPos;
   };

   class KBKey
   {
   public:
      KBKey() { col = row = -1; }

      int col;
      int row;
   };

   class KBAction
   {
   public:
      KBAction() { actionType = AT_INVALID; }

      typedef enum
      {
         AT_INVALID = 0,
         AT_SENDINPUT
      } ActionType;
      ActionType actionType;

      CString arg;
   };

   class KBEvent
   {
   public:
      int keyPosition;
      std::vector<KBKey> keys;
      std::vector<KBAction> actions;
   };

   class Config
   {
   public:
      Config(CXKeysDriverDlg& outer) : outer(outer)
      {
         cardReaderKeyPos = -1;
      }

      void LoadConfigFile(BOOL forceLoad = FALSE);

      CTime configFileLoadTime;

      std::vector<KBEvent> events;

      int cardReaderKeyPos;

      void TriggerEvents(KBState& s);
      void ExecuteEvent(KBEvent& e);

   private:
      CXKeysDriverDlg& outer;
   };

   Config cfg;
};
