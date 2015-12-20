// CaptureData.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include<stdlib.h>
#include<WinSock2.h>
#include<filesystem>
#include<mstcpip.h>

#pragma  comment (lib, "WS2_32.lib")//加载动态库 ws2_32.dll (system32)

#define OK	0
#define ERR	-1
#define MY_PORT	10000	//开端口一般用1024以上的端口，1024以下的很多被系统或程序占用

#define HOST_NAME_LEN	256
#define BUFF_LEN	65535

typedef struct IP_HEADER
{
	char cVer : 4;				//版本号
	char cHeadLen : 4;			//包头长度
	char cTOS;					//服务类型
	unsigned short usLen;		//包总长
	unsigned short usIdent;		//标识符
	unsigned short usFlagOffset;//标记（3位），片偏移（13位）
	char cTTL;					//生存时间	
	char cProtocol;				//协议
	unsigned short usChkSum;	//头部校验
	unsigned int   uiSrcIp;		//源地址
	unsigned int   uiDstIp;		//目标地址
}IP_HEADER_S;

void PrintData(char szBuf[], int len)
{
	IP_HEADER_S *pHeader;
	SOCKADDR_IN stAddr = { 0 };

	pHeader = (IP_HEADER_S *)szBuf;

	printf("\nRCV:\n");
	printf("Version:%d\n", pHeader->cVer);
	printf("IP len:%d\n", pHeader->cHeadLen);
	printf("TOS:%d\n", pHeader->cTOS);
	printf("Data Len:%d\n", ntohs(pHeader->usLen)); //注意字节序转化
	printf("TTL:%d\n", pHeader->cTTL);
	printf("Protocol:%d\n", pHeader->cProtocol);

	stAddr.sin_addr.s_addr = pHeader->uiSrcIp;  //stAddr.sin_addr.s_addr 存的是网络字节序的int
	//inet_ntoa 把 in_addr 转成字符串的ip地址
	printf("Source IP:%s\n", inet_ntoa(stAddr.sin_addr));

	stAddr.sin_addr.s_addr = pHeader->uiDstIp;
	printf("Dest IP:%s\n", inet_ntoa(stAddr.sin_addr));

	printf("\n");
}

int CaptureData()
{
	SOCKET iFd;
	char szHostName[HOST_NAME_LEN];	//计算机名
	HOSTENT *pHost;
	SOCKADDR_IN stAddr = { 0 };
	DWORD dwInBuff = 1;
	DWORD dwOutBuff;
	DWORD dwByteRet;
	int iRet;
	char szBuf[BUFF_LEN];

	WSADATA stWsa;					//用于打开动态链接库，是网络编程接口可用
	//0x0202版本号
	if (OK != WSAStartup(0x0202, &stWsa))
	{
		printf("Wsa startup failed.\n");
		return ERR;
	}

	//获取本机主机名
	if (OK != gethostname(szHostName, HOST_NAME_LEN))
	{
		WSACleanup();
		return ERR;
	}

	printf("hostName: %s\n", szHostName);
	pHost = gethostbyname(szHostName);				//通过主机名获取IP
	printf("MyIP:%d.%d.%d.%d\n", pHost->h_addr_list[0][0] & 0xff,
		pHost->h_addr_list[0][1] & 0xff,
		pHost->h_addr_list[0][2] & 0xff,
		pHost->h_addr_list[0][3] & 0xff);

	//整数表示的IP
	stAddr.sin_addr.s_addr = *(unsigned long*)(pHost->h_addr);
	//标准协议族
	stAddr.sin_family = AF_INET;
	//端口 网络是大端，要转换为小端
	stAddr.sin_port = htons(MY_PORT);

	//打开套接字
	//接受原始数据流，我们要接受的数据不仅仅是发往这个端口，而是所有端口
	iFd = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
	if (iFd < 0)
	{
		WSACleanup();
		return ERR;
	}

	if (OK != bind(iFd, (SOCKADDR*)&stAddr, sizeof(SOCKADDR)))
	{
		WSACleanup();
		closesocket(iFd);
		return ERR;
	}
	//设置iFd收包规则，SIO_RCVALL接受所有的包
	iRet = WSAIoctl(iFd, SIO_RCVALL, &dwInBuff, sizeof(dwInBuff), &dwOutBuff, sizeof(dwOutBuff),
		&dwByteRet, NULL, NULL);
	if (OK != iRet)
	{
		printf("IOCTL failed.\n");
		WSACleanup();
		closesocket(iFd);
		return ERR;
	}

	while (1)
	{
		memset(szBuf, 0, BUFF_LEN);
		iRet = recv(iFd, szBuf, BUFF_LEN, 0);
		if (iRet < 0)
		{
			break;
		}
		else
		{
			PrintData(szBuf, iRet);
		}
	}

	WSACleanup();
	return OK;

}

int _tmain(int argc, _TCHAR* argv[])
{
	CaptureData();
	system("pause");
	return 0;
}

