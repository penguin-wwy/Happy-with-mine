// HideInBMP.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<Windows.h>

char *ReadFileContent(char *filename, DWORD *filesize)
{
	/*HANDLE WINAPI CreateFile(
		_In_ LPCTSTR lpFileName,	��ͨ�ļ��������豸�ļ���
		_In_ DWORD dwDesiredAccess,����ģʽ��д/����
		_In_ DWORD dwShareMode,		����ģʽ
		_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,	ָ��ȫ���Ե�ָ��
		_In_ DWORD dwCreationDisposition,	��δ���
		_In_ DWORD dwFlagsAndAttributes,	�ļ�����
		_In_opt_ HANDLE hTemplateFile		���ڸ����ļ����
		);*/
	HANDLE hfile = CreateFileA(filename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);
	if (INVALID_HANDLE_VALUE == hfile)
	{
		printf("���ļ�%sʧ��", filename);
		return NULL;
	}
	DWORD dwRead;
	DWORD dwSize = GetFileSize(hfile, &dwRead);
	*filesize = dwSize;
	char *buf = new char[dwSize];
	RtlZeroMemory(buf, dwSize);
	//BOOL ReadFile(
	//	HANDLE hFile,    �ļ��ľ��
	//	LPVOID lpBuffer,   ���ڱ���������ݵ�һ��������
	//	DWORD nNumberOfBytesToRead,    Ҫ������ֽ���
	//	LPDWORD lpNumberOfBytesRead,    ָ��ʵ�ʶ�ȡ�ֽ�����ָ��
	//	LPOVERLAPPED lpOverlapped
	//	//���ļ���ʱָ����FILE_FLAG_OVERLAPPED����ô���룬�������������һ������Ľṹ��
	//	//�ýṹ������һ���첽��ȡ����������Ӧ�����������ΪNULL
	//	);
	ReadFile(hfile, buf, dwSize, &dwRead, 0);
	if (dwRead != dwSize)
	{
		printf("��ȡ�ļ�%s���ݳ���\n", filename);
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
		printf("���ļ�%sʧ��", filename);
		return NULL;
	}
	DWORD dwWrite;//����д�˶����ֽڵ��ļ���ȥ��
	/*SetFilePointer(
		_In_ HANDLE hFile,
		_In_ LONG lDistanceToMove,	�ֽ�ƫ����
		_Inout_opt_ PLONG lpDistanceToMoveHigh,ָ��һ�����������������а�����Ҫʹ�õ�һ����˫��ƫ�ơ�����Ϊ�㣨��������ΪByVal������ʾֻʹ��lDistanceToMove
		_In_ DWORD dwMoveMethod��������֮һ
		FILE_BEGIN lOffset����λ����Ϊ���ļ���ʼ����ʼ������һ��ƫ��
		FILE_CURRENT lOffset����λ����Ϊ�ӵ�ǰλ�ÿ�ʼ�����һ��ƫ��
		FILE_END lOffset����λ����Ϊ���ļ�β��ʼ�����һ��ƫ��
		);*/
	SetFilePointer(hfile, 0, 0, FILE_BEGIN);
	/*WriteFile(
		_In_ HANDLE hFile,
		_In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer,ָ��д���ļ��� ���ݻ�����
		_In_ DWORD nNumberOfBytesToWrite,Ҫд�����ݵ��ֽ���������д�����ֽڣ���ʾʲô����д�룬��������ļ��ġ���һ���޸�ʱ�䡱�����λ��Զ��ϵͳ�������ܵ���������65535���ֽ�����
		_Out_opt_ LPDWORD lpNumberOfBytesWritten,ʵ��д���ļ����ֽ��������˱������������ص� ��
		_Inout_opt_ LPOVERLAPPED lpOverlapped������ָ��FILE_FLAG_OVERLAPPED��ǰ���´��ļ�����������ͱ�������һ������Ľṹ���Ǹ��ṹ������һ���첽д���������򣬸ò���Ӧ��Ϊ�գ���������ΪByVal As Long����������ֵ��
		);*/
	WriteFile(hfile, buf, len, &dwWrite, 0);
	if (len != dwWrite)
	{
		printf("д��ʧ�ܣ�");
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
	printf("��һ�����ص�ƫ���ǣ�%d", *lpFirstPointOffset);
	char *pCurrentBMP = pBMP + *lpFirstPointOffset;
	char *pCurrentSecret = pSecretFile;
	//�����ļ��ж����Ҫ����
	*((DWORD *)pCurrentBMP) = dwSecretSize;//��һ�����غ�Ĺܵ������ļ���С
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
	printf("��һ������ƫ��: %d", *pFirstPoint);
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
	printf("���");
	return 0;
}

