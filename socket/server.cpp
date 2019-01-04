// video.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h" 
#include "decl.h" 
#include "cJSON.h"
#include "threadpool.h"
#include "task.h"
#include<iostream>
#include<WinSock2.h>
#include<Windows.h>

#pragma comment(lib, "ws2_32.lib")
#define CONNECT_NUM_MAX 10

using namespace std;


//bool login_ = false;

long sock_lRead(SOCKET sockfd, char *pData, long lReadLen)
{
	long lRead, lLeft, lReallyRead = 0;

	lLeft = lReadLen;
	while (1)
	{
		//lRead = read(sockfd, pData + lReadLen - lLeft, lLeft);
		lRead = recv(sockfd, pData + lReadLen - lLeft, lLeft, 0);
		if (0 > lRead)
			return -1;
		else if (0 == lRead)
			break;
		else
		{
			lReallyRead += lRead;
			lLeft -= lRead;
		}
	}

	return lReallyRead;
}

int _tmain(int argc, _TCHAR* argv[])
{
	//test time
// 	UINT_PTR dwTimerId;
// 	dwTimerId = SetTimer(NULL, 1, 1000, NULL);
// 	MSG msg;
// 	//unsigned int i = 0;
// 	while (GetMessage(&msg, NULL, 0, 0))
// 	{
// 		if (msg.message == WM_TIMER)
// 		{
// 			cout << "Do some thing!" << endl;
// 			KillTimer(NULL, dwTimerId);
// 			break;
// 		}
// 	}

	//test thread
	ThreadPool threadPool(2, 10);
#if 0
 	size_t i = 0;
 	for ( i = 0; i < 30; i++)
	{
		threadPool.QueueTaskItem(Task::Task1, (PVOID)i, TaskCallback::TaskCallback1);
	}
	threadPool.QueueTaskItem(Task::Task1, (PVOID)i, TaskCallback::TaskCallback1, TRUE);
#endif
	//加载套接字库
	WSADATA wsaData;
	int iRet = 0;
	BOOL bRet = FALSE;

#if 1
	iRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iRet != 0)
	{
		cout << "WSAStartup(MAKEWORD(2, 2), &wsaData) execute failed!" << endl;;
		return -1;
	}
	if (2 != LOBYTE(wsaData.wVersion) || 2 != HIBYTE(wsaData.wVersion))
	{
		WSACleanup();
		cout << "WSADATA version is not correct!" << endl;
		return -1;
	}

	//创建套接字
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		cout << "serverSocket = socket(AF_INET, SOCK_STREAM, 0) execute failed!" << endl;
		return -1;
	}
	
	//初始化服务器地址族变量
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	//绑定
	iRet = bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (iRet == SOCKET_ERROR)
	{
		cout << "bind(serverSocket, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR)) execute failed!" << endl;
		return -1;
	}

	//监听
	iRet = listen(serverSocket, CONNECT_NUM_MAX);
	if (iRet == SOCKET_ERROR)
	{
		cout << "listen(serverSocket, 10) execute failed!" << endl;
		return -1;
	}
	//等待连接_接收_发送
	SOCKADDR_IN clientAddr;
	int len = sizeof(SOCKADDR);
	while (1)
	{
		SOCKET connSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &len);
		if (connSocket == INVALID_SOCKET)
		{
			cout << "accept(serverSocket, (SOCKADDR*)&clientAddr, &len) execute failed!" << endl;
			return -1;
		}

		//char sendBuf[100];
		//sprintf_s(sendBuf, "Welcome %s", inet_ntoa(clientAddr.sin_addr));
		//send(connSocket, sendBuf, strlen(sendBuf) + 1, 0);
		//printf("SEND %d\n", __LINE__);
		char recvHead[32];
		char recvBuf[128];
		printf("%s -- %d\n", __FILE__, __LINE__);

		//先接收报文头
		memset(recvHead, 0, 32);
		memset(recvBuf, 0, 128);
		//recv(connSocket, recvHead, 16, 0);
		sock_lRead(connSocket, recvHead, 16);
		printf("%s\n", recvHead);
		if (strncmp(recvHead, "vd0010", strlen("vd0010")))
		{
			//error
			continue;
		}

		char lenStr[16];
		strcpy_s(lenStr, recvHead + 6);
		int bodyLen = atoi(lenStr);
		//recv(connSocket, recvBuf, bodyLen, 0);
		sock_lRead(connSocket, recvBuf, bodyLen);

		printf("%s\n", recvBuf);

		parmStructDef *pram1 = new  parmStructDef;
		pram1->sockets = connSocket;
		pram1->msgBuf = (char *)malloc(strlen(recvBuf) + 1);
		memset(pram1->msgBuf, 0, strlen(recvBuf) + 1);
		strcpy(pram1->msgBuf,  recvBuf);
		threadPool.QueueTaskItem(Task::parMsg, pram1, NULL);
		printf("%s -- %d\n", __FILE__, __LINE__);

		//closesocket(connSocket);
	}
//#else
	parmStructDef *pram1 = new  parmStructDef;
	pram1->sockets = 1;
	pram1->msgBuf = (char *)malloc(512);
	memset(pram1->msgBuf, 0, 512);
	strcpy(pram1->msgBuf, "{\"serialNum\":\"BLC20181022123059000111\",\"deviceID\":\"C73651963\",\"action\":\"2\",\"durations\":\"0\"}");
	threadPool.QueueTaskItem(Task::parMsg, pram1, NULL);
#endif
	system("pause");
	return 0;
}

