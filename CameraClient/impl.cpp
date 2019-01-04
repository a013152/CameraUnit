#include "stdafx.h"
#include "decl.h"


int m_iCurChanIndex = -1;               //��ǰͨ��������������
bool g_bShowDisplayWindow = false;
int iErrorCode = 0;//������
LONG lLoginID = -1;
HANDLE hThread;
DWORD dwThreadId;
char g_AppPath[256] = { 0 };
char szConten[256] = { 0 };
char szCameraIp[256] = { "172.16.0.74" };
char szCapturePath[256] = { 0 };  //��ͼ����·��
char szRecordePath[256] = { 0 };	//¼�񱣴�·��
TCHAR szCurChanIndex[256] = { 0 };
std::vector<stCamera> vtCamera;  //

std::vector<stCamera>& getVTCamera(){ return vtCamera; }

FILE *fpError = NULL; 

//��ʼ�� 
bool initCamera(int* iErrorCode)
{
	*iErrorCode = 0;
	BOOL temp1 = NET_DVR_Init();          //Init SDK
	if (temp1 == FALSE){
		printf("��ʼ������SDKʧ��\n")		;
		*iErrorCode = E_INI_HKSDK;
		return false;
	}
	//��ȡ����  
	readConfig(iErrorCode);
	if (*iErrorCode){
		return false;
	}
	else{
		//ѭ����¼һ��
		for (int i = 0; i < vtCamera.size(); i++)
		{
			lLoginID = NET_DVR_Login_V30(vtCamera[i].szIP, vtCamera[i].nPort, \
				vtCamera[i].szUSER, vtCamera[i].szPASSWORD, &(vtCamera[i].DeviceInfo)); 
			if (lLoginID != -1){
				vtCamera[i].m_struDeviceInfo.lLoginID = lLoginID;
				vtCamera[i].m_struDeviceInfo.iDeviceChanNum = vtCamera[i].DeviceInfo.byChanNum;
				vtCamera[i].m_struDeviceInfo.iIPChanNum = vtCamera[i].DeviceInfo.byIPChanNum;
				vtCamera[i].m_struDeviceInfo.iStartChan = vtCamera[i].DeviceInfo.byStartChan;
				vtCamera[i].m_struDeviceInfo.iIPStartChan = vtCamera[i].DeviceInfo.byStartDChan;
				vtCamera[i].strSerialNumber = (char*)vtCamera[i].DeviceInfo.sSerialNumber;  //����
				DoGetDeviceResoureCfg(vtCamera[i]);

				if (g_bShowDisplayWindow) { //��Ҫ��ʾ���ⴴ������
					if (vtCamera[i].m_hWnd == NULL){
						vtCamera[i].m_hWnd = CreatePlayWindow(MBytesToWString(vtCamera[i].m_struDeviceInfo.struChanInfo[0].chChanName).c_str(), iErrorCode);
					}
				}
				if (vtCamera[i].m_bIsPlaying == false)
				{
					getPlayHandle(vtCamera[i], vtCamera[i].m_struDeviceInfo.iStartChan);
				}
				if (vtCamera[i].m_bIsPlaying)  //���ڲ��ţ�ֹͣ����
				{
					NET_DVR_StopRealPlay(vtCamera[i].m_lPlayHandle);
					vtCamera[i].m_bIsPlaying = false;
				}
				if (vtCamera[i].m_bLogin)
					NET_DVR_Logout_V30(vtCamera[i].m_struDeviceInfo.lLoginID); 
			} 
		} 
	}
	return true;
}

//����ʼ��
void uninitCamera()
{
	for (int i = 0; i < vtCamera.size(); i++)
	{
		if (vtCamera[i].m_bIsRecording)  //����¼����ֹͣ
		{
			NET_DVR_StopSaveRealData(vtCamera[i].m_lPlayHandle);
		}
		if (vtCamera[i].m_bIsPlaying)  //���ڲ��ţ�ֹͣ����
		{
			NET_DVR_StopRealPlay(vtCamera[i].m_lPlayHandle);
			vtCamera[i].m_bIsPlaying = false;
		}
		if (vtCamera[i].m_bLogin)
		{
			NET_DVR_Logout_V30(vtCamera[i].m_struDeviceInfo.lLoginID);
			vtCamera[i].m_bLogin = false;
		}
		if (vtCamera[i].m_hWnd != NULL)
		{
			::PostMessage(vtCamera[i].m_hWnd, WM_CLOSE, 0, 0);
			vtCamera[i].m_hWnd = NULL;
		}
	} 
	NET_DVR_Cleanup();
	closeLog();
}



//��ȡ���ã������Ҫ��¼������ͷ��Ϣ��·����Ϣ��
void readConfig(int *iErrorCode)
{
	*iErrorCode = 0; 
	GetModuleFileNameA(NULL, g_AppPath, MAX_PATH);
	std::string  strPath = (std::string)g_AppPath;    // Get full path of the file   
	int pos = strPath.find_last_of('\\', strPath.length());
	strcpy_s(g_AppPath, strPath.substr(0, pos).c_str());
	strPath = strPath.substr(0, pos);
	strPath += "\\set.ini";
	if (_access(strPath.c_str(), 0) == -1){ //�ж��ļ��Ƿ����
		*iErrorCode = E_INI_FILE_NOT_EXIST;
		return;
	}
	g_bShowDisplayWindow = GetPrivateProfileIntA("set", "ShowDisplayWindow", 0, strPath.c_str()) == 1;

	int number_ = GetPrivateProfileIntA("devices", "number", 0, strPath.c_str());
	if (number_ == 0){
		*iErrorCode = E_INI_FILE_CAMER_NUMBER;
		return;
	}
	GetPrivateProfileStringA("set", "capturePath", "", szCapturePath, 256, strPath.c_str());
	GetPrivateProfileStringA("set", "recordePath", "", szRecordePath, 256, strPath.c_str());

	vtCamera.clear();
	//��ȡ����ͷ��Ϣ
	std::string strTemp; char szTemp[256] = { 0 }, szTemp2[256];
	std::vector<std::string> vtStrSub;
		
	for (int i = 0; i < number_; i++)
	{
		stCamera obj;
		sprintf_s(szTemp, "Camera%02d", i + 1);
		GetPrivateProfileStringA("devices", szTemp, "", szTemp2, 256, strPath.c_str());
		strTemp = szTemp2;
		splitString(strTemp, ',', vtStrSub);
		if (vtStrSub.size() >= 4){
			strcpy_s(obj.szIP, vtStrSub[0].c_str());
			obj.nPort = atoi(vtStrSub[1].c_str());
			strcpy_s(obj.szUSER, vtStrSub[2].c_str());
			strcpy_s(obj.szPASSWORD, vtStrSub[3].c_str());
			vtCamera.push_back(obj);
		}
	} 
}

//��¼����ͷ�����Ҵ�����Ҫ���Ŵ���
bool loginCamera(char* szCameraSerialNumber, int* iErrorCode)
{
	*iErrorCode = 0;
	if (vtCamera.size() == 0){
		*iErrorCode = E_INI_FILE_CAMER_NUMBER;
		return false;
	}
	LONG lUserID = 0; /*LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber, /*&lUserID, &lChannel, &lPlayHandle,*/ &index_)) //�ж����к��Ƿ����б��� 
	{
		lLoginID = NET_DVR_Login_V30(vtCamera[index_].szIP, vtCamera[index_].nPort, \
			vtCamera[index_].szUSER, vtCamera[index_].szPASSWORD, &(vtCamera[index_].DeviceInfo));
		if (lLoginID == -1)
		{
			logInfo(logType::errorLog, "��¼%s����ͷ�豸ʧ��,����SDK������%d!", vtCamera[index_].szIP, NET_DVR_GetLastError());
			printf("��¼%s����ͷ�豸ʧ��,����SDK������%d!\n", vtCamera[index_].szIP, NET_DVR_GetLastError());
			*iErrorCode = E_LOGIN_CAMERA;
		}
		else{
			printf("��¼%s����ͷ�豸 �ɹ�!loginID��%d ���кţ�%s\n", szCameraIp, lLoginID, vtCamera[index_].DeviceInfo.sSerialNumber);
			
			vtCamera[index_].m_struDeviceInfo.lLoginID = lLoginID;
				//vtCamera[index_].m_struDeviceInfo.iDeviceChanNum = vtCamera[index_].DeviceInfo.byChanNum;
				//vtCamera[index_].m_struDeviceInfo.iIPChanNum = vtCamera[index_].DeviceInfo.byIPChanNum;
				//vtCamera[index_].m_struDeviceInfo.iStartChan = vtCamera[index_].DeviceInfo.byStartChan;
				//vtCamera[index_].m_struDeviceInfo.iIPStartChan = vtCamera[index_].DeviceInfo.byStartDChan;
				//vtCamera[index_].strSerialNumber = (char*)vtCamera[index_].DeviceInfo.sSerialNumber;  //����
				//DoGetDeviceResoureCfg(vtCamera[index_]);

				//if (g_bShowDisplayWindow) { //��Ҫ��ʾ���ⴴ������
				//	if (vtCamera[index_].m_hWnd == NULL){
				//		vtCamera[index_].m_hWnd = CreatePlayWindow(MBytesToWString(vtCamera[index_].m_struDeviceInfo.struChanInfo[0].chChanName).c_str(), iErrorCode);
				//	}
				//} 
			
			if (vtCamera[index_].m_bIsPlaying == false)
			{
				getPlayHandle(vtCamera[index_], vtCamera[index_].m_struDeviceInfo.iStartChan);
			}
			vtCamera[index_].m_bLogin = true;
		}
	}
	else {
		*iErrorCode = E_NOT_SN;
		return false;
	}
	return true;
}

//�ǳ�����ͷ�� 
//����1 ����ͷ���к�
//����2 ��������� 
bool logoutCamera(char* szCameraSerialNumber, int* iErrorCode)
{
	/* LONG lUserID = 0;LONG lChannel = 0; LONG lPlayHandle = 0; */int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber, /*&lUserID, &lChannel, &lPlayHandle,*/ &index_)) //�ж����к��Ƿ����б��� 
	{
		if (vtCamera[index_].m_bIsPlaying == true)
		{
			NET_DVR_StopRealPlay(vtCamera[index_].m_lPlayHandle);
			vtCamera[index_].m_bIsPlaying = false;
		}
		if (vtCamera[index_].m_bLogin){
			NET_DVR_Logout_V30(vtCamera[index_].m_struDeviceInfo.lLoginID);
			vtCamera[index_].m_bLogin = false;
		}
	}else{
		*iErrorCode = E_NOT_SN;
		return false;
	}
	return true;
}

//����ͷ����
//����1 ����ͷ���к�
//����2 ͼƬ����·��
//����3 ��������� 
void cameraCapture(char* szCameraSerialNumber, char* szJpgPath, int* iErrorCode)
{
	*iErrorCode = 0;
	/*LONG lUserID = 0; LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber,/* &lUserID, &lChannel, &lPlayHandle,*/ &index_)) //�ж����к��Ƿ����б���
	{
		//�ж�·�����ļ����Ƿ����
		detectPath(szJpgPath);
		bool blogin = loginCamera(szCameraSerialNumber, iErrorCode);
		if (blogin){
			NET_DVR_JPEGPARA JpgPara = { 0 };		 JpgPara.wPicSize = 0;		 JpgPara.wPicQuality = 0;
		 
			if (NET_DVR_CaptureJPEGPicture(vtCamera[index_].m_struDeviceInfo.lLoginID,\
				vtCamera[index_].m_struDeviceInfo.struChanInfo[0].iChanIndex, &JpgPara, szJpgPath))
			{
				printf("��ͼ�ɹ���������%s\n", szJpgPath);
			}
			else{
				printf("��ͼʧ�ܣ�������%d\n", NET_DVR_GetLastError());
				*iErrorCode = NET_DVR_GetLastError();
			}
			logoutCamera(szCameraSerialNumber, iErrorCode);
		}		
	}
	else{
		*iErrorCode = E_NOT_SN;
	}
}

//����ͷ¼��
//����1 ����ͷ���к�
//����2 ¼���־   true:��ʼ\false:ֹͣ
//����3 MP4����·�� 
//����4 ��������� 
void cameraRecod(char* szCameraSerialNumber, bool recodFlag, char* szMP4Path, int* iErrorCode)
{
	*iErrorCode = 0;
	/*LONG lUserID = 0; LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber,/* &lUserID, &lChannel, &lPlayHandle,*/ &index_)) //�ж����к��Ƿ����б���
	{
		//�ж�·�����ļ����Ƿ����
		detectPath(szMP4Path); 

		if (recodFlag)  //����¼��
		{
			if (vtCamera[index_].m_bIsRecording == false){
				if (vtCamera[index_].m_bLogin == false){
					 loginCamera(szCameraSerialNumber, iErrorCode);   //�ȵ�¼
				}
				if (vtCamera[index_].m_bLogin == true ){  
					if (!NET_DVR_SaveRealData(vtCamera[index_].m_lPlayHandle, szMP4Path))
					{
						printf("����¼��ʧ�ܣ��������%d\n", NET_DVR_GetLastError());
						*iErrorCode = NET_DVR_GetLastError();
						logInfo(errorLog, "����¼��ʧ�ܣ��������%d", NET_DVR_GetLastError());
						logoutCamera(szCameraSerialNumber, iErrorCode);
					}
					else{
						printf("����¼��ɹ���·����%s�� �ۿ�ǰ��ֹͣ¼��\n", szMP4Path);
						vtCamera[index_].m_bIsRecording = true;
					}
				}

			}
			else{
				printf("%s����¼��\n", szCameraSerialNumber);
				*iErrorCode = E_RECODING;
			} 
		}
		else  //ֹͣ¼��
		{
			if (vtCamera[index_].m_bIsRecording == true){
				if (!NET_DVR_StopSaveRealData(vtCamera[index_].m_lPlayHandle))
				{
					printf("ֹͣ¼��ʧ�ܣ��������%d\n", NET_DVR_GetLastError());
					*iErrorCode = NET_DVR_GetLastError();
				}
				else{
					printf("ֹͣ¼��ɹ�.\n");
					vtCamera[index_].m_bIsRecording = false;
					if (vtCamera[index_].m_bLogin == true){
						logoutCamera(szCameraSerialNumber, iErrorCode);   //�ǳ�
					}
				}
			}
			else
			{
				printf("%sû������¼��\n", szCameraSerialNumber);
				*iErrorCode = E_NOT_RECODING;
			}
		}
	}
	else{
		*iErrorCode = E_NOT_SN;
	}
}



LRESULT CALLBACK WinSunProc(
	HWND hwnd,  // ���ڵľ��
	UINT uMsg,   //��Ϣ��ʶ��
	WPARAM wParam, // ��һ����Ϣ����
	LPARAM lParam  //�ڶ�����Ϣ����
	);  //�ص�����

DWORD WINAPI ThreadProFunc(LPVOID lpParam);

//����һ�����Ŵ���
HWND CreatePlayWindow(const TCHAR* szClassName, int* errorCode) 
{
	*errorCode = 0;
	WNDCLASS wndcls;    //ʵ����
	wndcls.cbClsExtra = 0;   //�������ĸ����ֽ���
	wndcls.cbWndExtra = 0;   //���ڶ���ĸ����ֽ���
	//HGDIOBJ GetStockObject(int fnObject);����Ԥ����ı��ñʡ�ˢ�ӡ�������ߵ�ɫ��ľ��
	//BLACK_BRUSH ָ��ɫ��ˢ
	wndcls.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);  //������ɫ��GetStockObject()�����ķ���ֵ��һ��HGDIOBJ��ʽ�ģ�ǿ��ת����HBRUSH��ʽ�ĸ�ֵ
	//LoadCursor����ָ���Ĺ������
	//ԭ�Σ�HCURSOR LoadCursor(HINSTANCE hlnstance��LPCTSTR lpCursorName);
	//hlnstance��ʶһ��ģ�����������Ŀ�ִ���ļ�����Ҫ����Ĺ��,lpCursorName�����Դ��
	wndcls.hCursor = LoadCursor(NULL, IDC_CROSS);  //�������� IDC_CROSS����ʮ�ֹ��
	// LoadIcon�����Ѿ���LoadImage���
	wndcls.hIcon = LoadIcon(NULL, IDI_ERROR);   //ͼ��
	wndcls.hInstance = NULL;//hInstance;              //Ӧ�ó����ʵ����
	wndcls.lpfnWndProc = WinSunProc;       //���ڹ��̺���
	wndcls.lpszClassName = szClassName;  //����
	wndcls.lpszMenuName = NULL;   //�˵���
	wndcls.style = CS_HREDRAW | CS_VREDRAW; //ˮƽ�ػ��ʹ�ֱ�ػ� 

	RegisterClass(&wndcls);   //ע�ᴰ���� 

	static int s_timer_x = 0; static int s_timer_y = 0;

	HWND hWnd = CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW, (s_timer_x % 3 * 400), (s_timer_y / 3 * 213), 600, 320, NULL, NULL, NULL, NULL); //����һ������
	ShowWindow(hWnd, SW_SHOWNORMAL);   //��ʾ�������ش���  
	UpdateWindow(hWnd);  //���´���
	s_timer_x++; s_timer_y++;

	static bool first_ = true;
	if (first_){
		hThread = CreateThread(NULL	// Ĭ�ϰ�ȫ����
			, NULL		// Ĭ�϶�ջ��С
			, ThreadProFunc // �߳���ڵ�ַ
			, NULL	//���ݸ��̺߳����Ĳ���
			, 0		// ָ���߳���������
			, &dwThreadId	//�߳�ID��
			);
		first_ = false;
	}

	return hWnd; 
}

DWORD WINAPI ThreadProFunc(LPVOID lpParam)
{
	MSG msg;   //��Ϣ����
	while (GetMessage(&msg, NULL, 0, 0))    //��Ϣѭ��
	{
		TranslateMessage(&msg);   //����������Ϣ����
		DispatchMessage(&msg);    //
	}
	return 0;
}

LRESULT CALLBACK WinSunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		//if (IDYES == MessageBox(hwnd, L"�Ƿ��˳���", L"tips", MB_YESNO))
	{
		DestroyWindow(hwnd);  //���ٴ���
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		CloseHandle(hThread);	//�ر��߳̾�����ں����ü�����һ
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void getPlayHandle(stCamera& vtCameraObj, int iChanIndex)  //��ȡ����ͨ��
{
	NET_DVR_CLIENTINFO ClientInfo;
	ClientInfo.hPlayWnd = vtCameraObj.m_hWnd;//������Ҫһ�����ھ�� 
	ClientInfo.lChannel = iChanIndex ;
	ClientInfo.lLinkMode = 0;
	ClientInfo.sMultiCastIP = NULL;
	vtCameraObj.m_lPlayHandle = NET_DVR_RealPlay_V30(vtCameraObj.m_struDeviceInfo.lLoginID, &ClientInfo, NULL, NULL, TRUE);
	if (-1 == vtCameraObj.m_lPlayHandle)
	{
		DWORD err = NET_DVR_GetLastError();
		printf("���ų����������%d\n", err);
		logInfo(errorLog, "���ų����������%d", err);
		return;
	}

	vtCameraObj.m_bIsPlaying = true;
}

/*���ֽ�ת���ֽ�*/
std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, strlen(lpcszString), NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, strlen(lpcszString), (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}


/*���ֽ�ת���ֽ�*/
std::string WStringToMBytes(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen;
	iTextLen = ::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, wcslen(lpwcszWString), NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, wcslen(lpwcszWString), pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}


//�ж����к��Ƿ���vector��
bool getSNmapInfo(char* szCameraSerialNumber,/* LONG* lUserID, LONG* lChannel, LONG* lPlayHandle,*/ int *index)
{
	bool flag = false;
	for (int i = 0; i < vtCamera.size(); i++)
	{ 
		if (vtCamera[i].strSerialNumber.find(szCameraSerialNumber) != -1)
		{
			/* *lUserID = vtCamera[i].m_struDeviceInfo.lLoginID;
			*lChannel = vtCamera[i].m_struDeviceInfo.struChanInfo[0].iChanIndex;
			*lPlayHandle = vtCamera[i].m_lPlayHandle;*/
			*index = i;
			flag = true;
			break;
		}
	}
	return flag;
}

//�ָ��ַ�
void splitString(std::string strtem, char a, std::vector<std::string>& vtStrSub)
{
	vtStrSub.clear();
	std::string::size_type pos1, pos2;
	pos2 = strtem.find(a);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		vtStrSub.push_back(strtem.substr(pos1, pos2 - pos1));
		pos1 = pos2 + 1;
		pos2 = strtem.find(a, pos1);
	}
	vtStrSub.push_back(strtem.substr(pos1));
}

//д��log��Ϣ
void logInfo(logType type_, const char* format, ...)
{
	static char szLogSumb[1024] = { 0 };
	static char szDstLan[1024] = { 0 };
	va_list arglist;
	va_start(arglist, format);
	vsprintf_s(szDstLan, format, arglist);
	va_end(arglist);

	 SYSTEMTIME st = { 0 };	 GetLocalTime(&st);
	 sprintf_s(szLogSumb, "%04d%02d%02d %02d%02d%02d: %s\n", st.wYear, st.wMonth, st.wDay, \
		 st.wHour, st.wMinute, st.wSecond, szDstLan); 

	if (type_ == errorLog){
		if (fpError == NULL){
			char szTemp[256] = { 0 };
			sprintf_s(szTemp, "%s\\log\\eLog.txt",g_AppPath);
			fopen_s(&fpError,szTemp, "ab+");/*�Զ�/д��ʽ��һ���������ļ�������������ļ�ĩ׷�����ݡ����������,���Զ�����*/
		}
		if (fpError){
			fwrite(szLogSumb, strlen(szLogSumb), 1, fpError);
		}
	}
}

//�ر�log�ļ��߱�
void closeLog(){
	if (fpError){
		fclose(fpError);
	}
}

void DoGetDeviceResoureCfg(stCamera& vtCameraObj)
{
	NET_DVR_IPPARACFG_V40 IpAccessCfg;
	memset(&IpAccessCfg, 0, sizeof(IpAccessCfg));
	DWORD  dwReturned;

	vtCameraObj.m_struDeviceInfo.bIPRet = NET_DVR_GetDVRConfig(vtCameraObj.m_struDeviceInfo.lLoginID, NET_DVR_GET_IPPARACFG_V40, 0, &IpAccessCfg, sizeof(NET_DVR_IPPARACFG_V40), &dwReturned);

	int i;
	if (!vtCameraObj.m_struDeviceInfo.bIPRet)   //��֧��ip����,9000�����豸��֧�ֽ���ģ��ͨ��
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)
		{
			if (i < vtCameraObj.m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i].chChanName, "camera%d", i + vtCameraObj.m_struDeviceInfo.iStartChan);
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].iChanIndex = i + vtCameraObj.m_struDeviceInfo.iStartChan;  //ͨ����
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;

			}
			else
			{
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i].chChanName, "");
			}
		}
	}
	else        //֧��IP���룬9000�豸
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)  //ģ��ͨ��
		{
			if (i < vtCameraObj.m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i].chChanName, "camera%d", i + vtCameraObj.m_struDeviceInfo.iStartChan);
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].iChanIndex = i + vtCameraObj.m_struDeviceInfo.iStartChan;
				if (IpAccessCfg.byAnalogChanEnable[i])
				{
					vtCameraObj.m_struDeviceInfo.struChanInfo[i].bEnable = TRUE;
				}
				else
				{
					vtCameraObj.m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				}

			}
			else//clear the state of other channel
			{
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].iChanIndex = -1;
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].bEnable = FALSE;
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i].chChanName, "");
			}
		}

		//����ͨ��
		for (i = 0; i < MAX_IP_CHANNEL; i++)
		{
			if (IpAccessCfg.struStreamMode[i].uGetStream.struChanInfo.byEnable)  //ipͨ������
			{
				vtCameraObj.m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = TRUE;
				vtCameraObj.m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = i + IpAccessCfg.dwStartDChan;
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].chChanName, "IP Camera %d", i + 1);

			}
			else
			{
				vtCameraObj.m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].bEnable = FALSE;
				vtCameraObj.m_struDeviceInfo.struChanInfo[i + MAX_ANALOG_CHANNUM].iChanIndex = -1;
			}
		} 
	}
}

//̽���ļ�·�������������·�����򴴽�
void detectPath(const char* path_)  
{
	std::string strPath = path_;
	int pos = strPath.find_last_of('\\', strPath.length());
	if (pos == -1)
		return;
	strPath = strPath.substr(0, pos); 
	detectPath(strPath.c_str());
	if (_access(strPath.c_str(), 0) == -1){ //�ж��ļ��Ƿ����
		CreateDirectoryA(strPath.c_str(), NULL);
	}

}