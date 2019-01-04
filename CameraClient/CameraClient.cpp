// CameraClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include <windows.h>
#include <string>
#include "decl.h" 

int input_ =0;
char m_csErr[256] = { 0 };
char* szSerialNumber = "C73651963"; 

int disPlayCommand(){
	printf("请输入命令： 0 退出 \t1 截图 \t2 录像 \n");
	scanf_s("%d", &input_);
	return input_;
}
int _testmain(int argc, _TCHAR* argv[])
{
	bool temp1 = initCamera(&iErrorCode);  
	if (temp1){ 
		//3 循环接收命令
		while (TRUE)
		{
			//4处理命令
			switch (disPlayCommand())
			{
			case 0:  //退出
				uninitCamera(); 
				printf("退出登录\n");
				goto ENDPOINT;
				break;
			case 1:	//截图
			{ 
				 char szPath[256] = { 0 }; 
				 SYSTEMTIME st = { 0 };	 GetLocalTime(&st);
				 sprintf_s(szPath, "%s\\%04d-%02d-%02d %02d%02d%02d.jpg", szCapturePath, st.wYear, st.wMonth, st.wDay, \
					 st.wHour, st.wMinute, st.wSecond);
				 cameraCapture(szSerialNumber, szPath, &iErrorCode);
			}			
				break;
			case 2: //录像
			{
					  bool recodFlag = false;
					  REINPUT_RECORD:
					  printf("请输入：1开始录像，2停止录像,0返回上级\n");
					  scanf_s("%d", &input_);
					 
					  if (input_ == 1){
						  recodFlag = true;
					  }
					  else if (input_ == 2){
						  recodFlag = false;
					  }
					  else if (input_ == 0)
					  {
						  break;
					  }
					  else{
						  printf("未知命令，请重新输入\n");
						  goto REINPUT_RECORD;
					  }
					  char szPath[256] = { 0 };
					  SYSTEMTIME st = { 0 };	 GetLocalTime(&st);
					  sprintf_s(szPath, "%s\\%04d%02d%02d%02d%02d%02d.mp4", szRecordePath, st.wYear, st.wMonth, st.wDay, \
						  st.wHour, st.wMinute, st.wSecond);
					  cameraRecod(szSerialNumber, recodFlag,szPath,  &iErrorCode);
			}
				break;  
			default:
				printf("未知命令，请重新输入。\n");
				break;
			} 
		} 
	} 
	ENDPOINT:  
	NET_DVR_Cleanup();
	return 0;
}

