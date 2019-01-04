#include "stdafx.h"
#include "task.h"
#include "cJSON.h"
#include "decl.h" 
extern "C"
{
	#include "parJSON.h"
}
#include<iostream>

using namespace std;

int Task::parMsg(PVOID parm)
{
	char *value[5] = { 0 };
	char szPath[256] = { 0 };
	char staStr[256] = { 0 };
	SYSTEMTIME st = { 0 };

	int iRet = 0;
	int i = 0;

	parmStructDef *p = (parmStructDef *)parm;
	cJSON *json = cJSON_Parse(p->msgBuf);
	printf("%d\n", p->sockets);
	printf("%s\n", p->msgBuf);

	iRet = parJson(json, value);

	//向json中写入状态
	memset(staStr, 0, 256);
	sprintf(staStr, "%d", iRet);
	cJSON_AddItemToObject(json, "status", cJSON_CreateString(staStr));

	//向json中写入备注
	memset(staStr, 0, 256);
	switch (iRet)
	{
		case E_SUCCESS:
			sprintf(staStr, "success");
			break;
		case E_FAILD:
			sprintf(staStr, "Parse json faild!");
			break;
		case E_NOT_JSON:
			sprintf(staStr, "Request message is not a valid json format!");
			break;
		case E_GET_VALUE:
			sprintf(staStr, "Parse json to get key value faild!");
			break;
		case E_UNHAND_REQ:
			sprintf(staStr, "Unkown action type!");
			break;
		case E_ILLEGAL_V:
			sprintf(staStr, "action and durations need to be digital!");
			break;
		default:
			break;
	}
	cJSON_AddItemToObject(json, "remark", cJSON_CreateString(staStr));

	//编辑响应报文
	char *jsonStr = cJSON_Print(json);
	char *rspBuf = (char *)malloc(strlen(jsonStr) + 16 + 1);
	memset(rspBuf, 0, strlen(jsonStr) + 16 + 1);
	sprintf(rspBuf, "vd0011%010ld%s", strlen(jsonStr), jsonStr);

	//发送响应报文
	long lSend, lWrite = 0;
	long lLen = strlen(rspBuf);
	char *pData = rspBuf;

	while (lWrite < lLen)
	{
		//lSend = write(p->sockets, pData + lWrite, lLen - lWrite);
		lSend = send(p->sockets, pData + lWrite, lLen - lWrite, 0);
		printf("write %ld send %ld\n", lWrite, lSend);
		if (0 > lSend)
		{
			printf("errno %d\n", errno);
			if (EINTR == errno)
			{
				printf("send data interrupted!\n");
				return -1;
			}
			else
			{
				printf("Send Data Failed!\n");
				return -1;
			}
		}
		lWrite += lSend;
	}

	closesocket(p->sockets);

	//连接摄像头
	bool temp1 = initCamera(&iErrorCode);
	int recordTime = 0;
	MSG msg;
	//unsigned int i = 0;

	switch (atoi(value[F_ACTION_R]))
	{
		case R_CAM_CAP:
			GetLocalTime(&st);
			sprintf_s(szPath, "%s\\%s%04d-%02d-%02d %02d%02d%02d%d.jpg", szCapturePath, value[F_DEVICEID_R], st.wYear, st.wMonth, st.wDay, \
				st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
			cameraCapture(value[F_DEVICEID_R], szPath, &iErrorCode);
			break;
		case R_CAM_REC:
			GetLocalTime(&st);
			sprintf_s(szPath, "%s\\%04d%02d%02d%02d%02d%02d.mp4", szRecordePath, st.wYear, st.wMonth, st.wDay, \
				st.wHour, st.wMinute, st.wSecond);

			//开始录像
			cameraRecod(value[F_DEVICEID_R], true, szPath, &iErrorCode);

			//定时器
			UINT_PTR dwTimerId;

			if (value[F_DURATION_R] == NULL || atoi(value[F_DURATION_R]) == 0)
				recordTime = 1;				//默认录像一分钟
			else
				recordTime = atoi(value[F_DURATION_R]);

			dwTimerId = SetTimer(NULL, 1, 10000, NULL);
			
			while (GetMessage(&msg, NULL, 0, 0))
			{
				if (msg.message == WM_TIMER)
				{
					//关闭录像
					cameraRecod(value[F_DEVICEID_R], false, szPath, &iErrorCode);
					KillTimer(NULL, dwTimerId);
					break;
				}
			}

			break;
		default:
			cout << "Unkown request type!" << endl;
			break;
	}

	//退出摄像头
	uninitCamera();

	while (i < F_NUM_R)
	{
		if (value[i] != NULL)
		{
			free((void *)value[i]);
			value[i] = NULL;
		}

		i++;
	}
	return 0;
}