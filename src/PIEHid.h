#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

typedef enum {
	piNone = 0,
	piNewData = 1,
	piDataChange = 2
} EEventPI;

#define MAX_XKEY_DEVICES		128
#define PI_VID					0x5F3

typedef DWORD (__stdcall *PHIDDataEvent)(UCHAR *pData, DWORD deviceID, DWORD error);
typedef DWORD (__stdcall *PHIDErrorEvent)(long status);

extern "C" DWORD __stdcall EnumeratePIE(long VID, long *data, long &count);
extern "C" DWORD __stdcall GetXKeyVersion(long hnd);
extern "C" DWORD __stdcall SetupInterface(long hnd);
extern "C" DWORD __stdcall SetupInterfaceEx(long hnd, BOOL suppressDuplicateReports);
extern "C" VOID  __stdcall CloseInterface(long hnd);
extern "C" VOID  __stdcall CleanupInterface(long hnd);
extern "C" DWORD __stdcall ReadData(long hnd, UCHAR *data);
extern "C" DWORD __stdcall BlockingReadData(long hnd, UCHAR *data, int maxMillis);
extern "C" DWORD __stdcall WriteData(long hnd, UCHAR *data);
extern "C" DWORD __stdcall FastWrite(long hnd, UCHAR *data);
extern "C" DWORD __stdcall ReadLast(long hnd, UCHAR *data);
extern "C" DWORD __stdcall ClearBuffer(long hnd);
extern "C" DWORD __stdcall GetReadLength(long hnd);
extern "C" DWORD __stdcall GetWriteLength(long hnd);
extern "C" DWORD __stdcall SetDataCallback(long hnd, int eventType, PHIDDataEvent pDataEvent);
extern "C" DWORD __stdcall SetDataCallbackEx(long hnd, int eventType, PHIDDataEvent pEventCall, BOOL skipReportId);
extern "C" DWORD __stdcall SetErrorCallback(long hnd, PHIDErrorEvent pErrorCall);
extern "C" void __stdcall DongleCheck(int k0, int k1, int k2, int k3, int n0, int n1, int n2, int n3, int &r0, int &r1, int &r2, int &r3);
extern "C" void __stdcall DongleCheck2(int k0, int k1, int k2, int k3, int n0, int n1, int n2, int n3, int &r0, int &r1, int &r2, int &r3);
