// CameraClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <windows.h>
#include <string>
#include "decl.h" 

int input_ =0;
char m_csErr[256] = { 0 };
char* szSerialNumber = "C73651963"; 

int disPlayCommand(){
	printf("��������� 0 �˳� \t1 ��ͼ \t2 ¼�� \n");
	scanf_s("%d", &input_);
	return input_;
}
int _testmain(int argc, _TCHAR* argv[])
{
	bool temp1 = initCamera(&iErrorCode);  
	if (temp1){ 
		//3 ѭ����������
		while (TRUE)
		{
			//4��������
			switch (disPlayCommand())
			{
			case 0:  //�˳�
				uninitCamera(); 
				printf("�˳���¼\n");
				goto ENDPOINT;
				break;
			case 1:	//��ͼ
			{ 
				 char szPath[256] = { 0 }; 
				 SYSTEMTIME st = { 0 };	 GetLocalTime(&st);
				 sprintf_s(szPath, "%s\\%04d-%02d-%02d %02d%02d%02d.jpg", szCapturePath, st.wYear, st.wMonth, st.wDay, \
					 st.wHour, st.wMinute, st.wSecond);
				 cameraCapture(szSerialNumber, szPath, &iErrorCode);
			}			
				break;
			case 2: //¼��
			{
					  bool recodFlag = false;
					  REINPUT_RECORD:
					  printf("�����룺1��ʼ¼��2ֹͣ¼��,0�����ϼ�\n");
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
						  printf("δ֪�������������\n");
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
				printf("δ֪������������롣\n");
				break;
			} 
		} 
	} 
	ENDPOINT:  
	NET_DVR_Cleanup();
	return 0;
}

