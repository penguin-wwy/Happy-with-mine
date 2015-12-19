// HideInBMP.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<Windows.h>

char *ReadFileContent(char *filename, DWORD *filesize)
{
	/*HANDLE WINAPI CreateFile(
		_In_ LPCTSTR lpFileName,	普通文件名或者设备文件名
		_In_ DWORD dwDesiredAccess,访问模式（写/读）
		_In_ DWORD dwShareMode,		共享模式
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,	指向安全属性的指针
		_In_ DWORD dwCreationDisposition,	如何创建
		_In_ DWORD dwFlagsAndAttributes,	文件属性
		_In_opt_ HANDLE hTemplateFile		用于复制文件句柄
		);*/
	HANDLE hfile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		printf("打开文件%s失败", filename);
		return NULL;
	}
	DWORD dwRead;
	DWORD dwSize = GetFileSize(hfile, &dwRead);
	*filesize = dwSize;
	char *buf = new char[dwSize];
	RtlZeroMemory(buf, dwSize);
	//BOOL ReadFile(
	//	HANDLE hFile,    文件的句柄
	//	LPVOID lpBuffer,   用于保存读入数据的一个缓冲区
	//	DWORD nNumberOfBytesToRead,    要读入的字节数
	//	LPDWORD lpNumberOfBytesRead,    指向实际读取字节数的指针
	//	LPOVERLAPPED lpOverlapped
	//	//如文件打开时指定了FILE_FLAG_OVERLAPPED，那么必须，用这个参数引用一个特殊的结构。
	//	//该结构定义了一次异步读取操作。否则，应将这个参数设为NULL
	//	);
	ReadFile(hfile, buf, dwSize, &dwRead, 0);
	if (dwRead != dwSize)
	{
		printf("读取文件%s内容出错\n", filename);
		return NULL;
	}
	CloseHandle(hfile);
	return buf;
}

bool SaveFile(char *filename, char *buf, int len)
{
	HANDLE hfile = CreateFileA(filename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		printf("打开文件%s失败", filename);
		return NULL;
	}
	DWORD dwWrite;//保存写了多少字节到文件中去了
	/*SetFilePointer(
		_In_ HANDLE hFile,
		_In_ LONG lDistanceToMove,	字节偏移量
		_Inout_opt_ PLONG lpDistanceToMoveHigh,指定一个长整数变量，其中包含了要使用的一个高双字偏移。可设为零（将声明变为ByVal），表示只使用lDistanceToMove
		_In_ DWORD dwMoveMethod下述常数之一
		FILE_BEGIN lOffset将新位置设为从文件起始处开始算的起的一个偏移
		FILE_CURRENT lOffset将新位置设为从当前位置开始计算的一个偏移
		FILE_END lOffset将新位置设为从文件尾开始计算的一个偏移
		);*/
	SetFilePointer(hfile, 0, 0, FILE_BEGIN);
	/*WriteFile(
		_In_ HANDLE hFile,
		_In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer,指向将写入文件的 数据缓冲区
		_In_ DWORD nNumberOfBytesToWrite,要写入数据的字节数量。如写入零字节，表示什么都不写入，但会更新文件的“上一次修改时间”。针对位于远程系统的命名管道，限制在65535个字节以内
		_Out_opt_ LPDWORD lpNumberOfBytesWritten,实际写入文件的字节数量（此变量是用来返回的 ）
		_Inout_opt_ LPOVERLAPPED lpOverlapped倘若在指定FILE_FLAG_OVERLAPPED的前提下打开文件，这个参数就必须引用一个特殊的结构。那个结构定义了一次异步写操作。否则，该参数应置为空（将声明变为ByVal As Long，并传递零值）
		);*/
	WriteFile(hfile, buf, len, &dwWrite, 0);
	if (len != dwWrite)
	{
		printf("写入失败！");
		return NULL;
	}
	CloseHandle(hfile);
	return true;
}

bool Hide(char *secretFileName, char *bmpFileName)
{
	DWORD dwBMPSize, dwSecretSize;
	char *pBMP = ReadFileContent(bmpFileName, &dwBMPSize);
	char *pSecretFile = ReadFileContent(secretFileName, &dwSecretSize);
	DWORD *lpFirstPointOffset = (DWORD *)(pBMP + 10);
	printf("第一个像素的偏移是：%d", *lpFirstPointOffset);
	char *pCurrentBMP = pBMP + *lpFirstPointOffset;
	char *pCurrentSecret = pSecretFile;
	//保存文件有多大，重要参数
	*((DWORD *)pCurrentBMP) = dwSecretSize;//第一个像素后的管道保存文件大小
	pCurrentBMP += 6;
	for (; pCurrentBMP < (pBMP + dwBMPSize) && pCurrentSecret <= (pSecretFile + dwSecretSize); pCurrentBMP += 6)
	{
		*pCurrentBMP = *pCurrentSecret;
		*(pCurrentBMP + 1) = *(pCurrentSecret + 1);
		*(pCurrentBMP + 2) = *(pCurrentSecret + 2);
		pCurrentSecret += 3;
	}
	SaveFile(bmpFileName, pBMP, dwBMPSize);
	delete[] pBMP;
	delete[] pSecretFile;

	return true;
}

bool Recovery(char *bmpFileName, char *secretFileName)
{
	DWORD dwBMPSize;
	char *pBMP = ReadFileContent(bmpFileName, &dwBMPSize);
	DWORD *pFirstPoint = (DWORD *)(pBMP + 10);
	printf("第一个像素偏移: %d", *pFirstPoint);
	DWORD dwSecretSize = *(DWORD *)(pBMP + *pFirstPoint + 3);
	char *buf = new char[dwSecretSize];
	char *pCurrentBMP = pBMP + *pFirstPoint + 6;
	for (int i = 0; pCurrentBMP < pBMP + dwBMPSize; pCurrentBMP += 6)
	{
		buf[i] = *pCurrentBMP;
		buf[i + 1] = *(pCurrentBMP + 1);
		buf[i + 2] = *(pCurrentBMP + 2);
		i += 3;
	}
	SaveFile(secretFileName, buf, dwSecretSize);
	delete[] pBMP;
	delete[] buf;

	return true;
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s Encrypt secret_file_name BMP_filename\n", argv[0]);
		printf("Usage: %s Decrypt BMP_filename secret_file_name\n", argv[0]);
	}
	if (0 == strcmp(argv[1], "Encrypt"))
	{
		Hide(argv[2], argv[3]);
	}
	else if (0 == strcmp(argv[1], "Decrypt"))
	{
		Recovery(argv[2], argv[3]);
	}
	else
		printf("Error");
	printf("完成");
	return 0;
}

