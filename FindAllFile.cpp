// FindeAllFile.cpp : �������̨Ӧ�ó������ڵ㡣
//
//#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#define	LEN	256	

BOOL DirectoryList(const TCHAR * path)
{
	WIN32_FIND_DATA FindData;
	HANDLE hError = INVALID_HANDLE_VALUE;
	int FileCount = 0;
	TCHAR FilePathName[LEN];

	//����·��
	TCHAR FullPathName[LEN];
	wcscpy_s(FilePathName, path);
	wcscat_s(FilePathName, _T("\\*.*"));
	hError = FindFirstFile(FilePathName, &FindData);
	if (hError == INVALID_HANDLE_VALUE){
		printf("����ʧ��!");
		return 0;
	}

	while (::FindNextFile(hError, &FindData)){
		// ����.��..
		if (wcscmp(FindData.cFileName, _T(".")) == 0 || wcscmp(FindData.cFileName, _T("..")) == 0)
			continue;
		//��������·��
		wsprintf(FullPathName, _T("%s\\%s"), path, FindData.cFileName);
		FileCount++;
		//��������ļ�
		OutputDebugString(FullPathName);
		_tprintf(_T("\n%d  %s"), FileCount, FullPathName);

		if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			printf("<Dir>");
			DirectoryList(FullPathName);
		}
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	DirectoryList(argv[1]);
	system("pause");
	return 0;
}

