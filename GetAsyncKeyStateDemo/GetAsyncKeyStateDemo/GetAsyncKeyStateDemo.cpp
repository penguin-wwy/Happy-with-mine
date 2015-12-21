// GetAsyncKeyStateDemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<Windows.h>
#include<iostream>

typedef BOOL(*Func)();
int _tmain(int argc, _TCHAR* argv[])
{
	DWORD dwError = NOERROR;
	BOOL bRet = FALSE;
	HINSTANCE hDll = nullptr;
	do{
		//需要在动态链接库实现全局的Hook
		//因为只有dll才是全局共享的
		hDll = LoadLibrary(L"SetWindowsHookDll.dll");
		if (!hDll)
		{
			dwError = GetLastError();
			break;
		}
		Func fun = (Func)GetProcAddress(hDll, "StartHook");
		if (!fun)
		{
			dwError = GetLastError();
			break;
		}
		if (!fun())
		{
			dwError = GetLastError();
			break;
		}

		while (true)
		{
			char szInput[MAXBYTE] = { 0 };
			std::cin >> szInput;
			if (0 == strcmp(szInput, "yes"))
			{
				break;
			}
		}

		bRet = TRUE;
	} while (false);
	
	if (FALSE == bRet)
		std::cout << "出现错误，错误编号:" << dwError << std::endl;
	
	if (hDll)
	{
		FreeLibrary(hDll);
	}
	
	system("pause");
	return 0;
}

