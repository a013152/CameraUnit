#include "stdafx.h"
#include "decl.h"


int m_iCurChanIndex = -1;               //当前通道在数组中索引
bool g_bShowDisplayWindow = false;
int iErrorCode = 0;//错误码
LONG lLoginID = -1;
HANDLE hThread;
DWORD dwThreadId;
char g_AppPath[256] = { 0 };
char szConten[256] = { 0 };
char szCameraIp[256] = { "172.16.0.74" };
char szCapturePath[256] = { 0 };  //截图保存路径
char szRecordePath[256] = { 0 };	//录像保存路径
TCHAR szCurChanIndex[256] = { 0 };
std::vector<stCamera> vtCamera;  //

std::vector<stCamera>& getVTCamera(){ return vtCamera; }

FILE *fpError = NULL; 

//初始化 
bool initCamera(int* iErrorCode)
{
	*iErrorCode = 0;
	BOOL temp1 = NET_DVR_Init();          //Init SDK
	if (temp1 == FALSE){
		printf("初始化海康SDK失败\n")		;
		*iErrorCode = E_INI_HKSDK;
		return false;
	}
	//读取配置  
	readConfig(iErrorCode);
	if (*iErrorCode){
		return false;
	}
	else{
		//循环登录一次
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
				vtCamera[i].strSerialNumber = (char*)vtCamera[i].DeviceInfo.sSerialNumber;  //保存
				DoGetDeviceResoureCfg(vtCamera[i]);

				if (g_bShowDisplayWindow) { //需要显示，测创建窗口
					if (vtCamera[i].m_hWnd == NULL){
						vtCamera[i].m_hWnd = CreatePlayWindow(MBytesToWString(vtCamera[i].m_struDeviceInfo.struChanInfo[0].chChanName).c_str(), iErrorCode);
					}
				}
				if (vtCamera[i].m_bIsPlaying == false)
				{
					getPlayHandle(vtCamera[i], vtCamera[i].m_struDeviceInfo.iStartChan);
				}
				if (vtCamera[i].m_bIsPlaying)  //正在播放，停止播放
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

//反初始化
void uninitCamera()
{
	for (int i = 0; i < vtCamera.size(); i++)
	{
		if (vtCamera[i].m_bIsRecording)  //正在录像，先停止
		{
			NET_DVR_StopSaveRealData(vtCamera[i].m_lPlayHandle);
		}
		if (vtCamera[i].m_bIsPlaying)  //正在播放，停止播放
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



//读取配置，获得需要登录的摄像头信息、路径信息等
void readConfig(int *iErrorCode)
{
	*iErrorCode = 0; 
	GetModuleFileNameA(NULL, g_AppPath, MAX_PATH);
	std::string  strPath = (std::string)g_AppPath;    // Get full path of the file   
	int pos = strPath.find_last_of('\\', strPath.length());
	strcpy_s(g_AppPath, strPath.substr(0, pos).c_str());
	strPath = strPath.substr(0, pos);
	strPath += "\\set.ini";
	if (_access(strPath.c_str(), 0) == -1){ //判断文件是否存在
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
	//读取摄像头信息
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

//登录摄像头，并且创建需要播放窗口
bool loginCamera(char* szCameraSerialNumber, int* iErrorCode)
{
	*iErrorCode = 0;
	if (vtCamera.size() == 0){
		*iErrorCode = E_INI_FILE_CAMER_NUMBER;
		return false;
	}
	LONG lUserID = 0; /*LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber, /*&lUserID, &lChannel, &lPlayHandle,*/ &index_)) //判断序列号是否在列表中 
	{
		lLoginID = NET_DVR_Login_V30(vtCamera[index_].szIP, vtCamera[index_].nPort, \
			vtCamera[index_].szUSER, vtCamera[index_].szPASSWORD, &(vtCamera[index_].DeviceInfo));
		if (lLoginID == -1)
		{
			logInfo(logType::errorLog, "登录%s摄像头设备失败,海康SDK错误码%d!", vtCamera[index_].szIP, NET_DVR_GetLastError());
			printf("登录%s摄像头设备失败,海康SDK错误码%d!\n", vtCamera[index_].szIP, NET_DVR_GetLastError());
			*iErrorCode = E_LOGIN_CAMERA;
		}
		else{
			printf("登录%s摄像头设备 成功!loginID：%d 序列号：%s\n", szCameraIp, lLoginID, vtCamera[index_].DeviceInfo.sSerialNumber);
			
			vtCamera[index_].m_struDeviceInfo.lLoginID = lLoginID;
				//vtCamera[index_].m_struDeviceInfo.iDeviceChanNum = vtCamera[index_].DeviceInfo.byChanNum;
				//vtCamera[index_].m_struDeviceInfo.iIPChanNum = vtCamera[index_].DeviceInfo.byIPChanNum;
				//vtCamera[index_].m_struDeviceInfo.iStartChan = vtCamera[index_].DeviceInfo.byStartChan;
				//vtCamera[index_].m_struDeviceInfo.iIPStartChan = vtCamera[index_].DeviceInfo.byStartDChan;
				//vtCamera[index_].strSerialNumber = (char*)vtCamera[index_].DeviceInfo.sSerialNumber;  //保存
				//DoGetDeviceResoureCfg(vtCamera[index_]);

				//if (g_bShowDisplayWindow) { //需要显示，测创建窗口
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

//登出摄像头， 
//参数1 摄像头序列号
//参数2 输出错误码 
bool logoutCamera(char* szCameraSerialNumber, int* iErrorCode)
{
	/* LONG lUserID = 0;LONG lChannel = 0; LONG lPlayHandle = 0; */int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber, /*&lUserID, &lChannel, &lPlayHandle,*/ &index_)) //判断序列号是否在列表中 
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

//摄像头拍照
//参数1 摄像头序列号
//参数2 图片保存路径
//参数3 输出错误码 
void cameraCapture(char* szCameraSerialNumber, char* szJpgPath, int* iErrorCode)
{
	*iErrorCode = 0;
	/*LONG lUserID = 0; LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber,/* &lUserID, &lChannel, &lPlayHandle,*/ &index_)) //判断序列号是否在列表中
	{
		//判断路径的文件夹是否存在
		detectPath(szJpgPath);
		bool blogin = loginCamera(szCameraSerialNumber, iErrorCode);
		if (blogin){
			NET_DVR_JPEGPARA JpgPara = { 0 };		 JpgPara.wPicSize = 0;		 JpgPara.wPicQuality = 0;
		 
			if (NET_DVR_CaptureJPEGPicture(vtCamera[index_].m_struDeviceInfo.lLoginID,\
				vtCamera[index_].m_struDeviceInfo.struChanInfo[0].iChanIndex, &JpgPara, szJpgPath))
			{
				printf("截图成功，保存在%s\n", szJpgPath);
			}
			else{
				printf("截图失败，错误码%d\n", NET_DVR_GetLastError());
				*iErrorCode = NET_DVR_GetLastError();
			}
			logoutCamera(szCameraSerialNumber, iErrorCode);
		}		
	}
	else{
		*iErrorCode = E_NOT_SN;
	}
}

//摄像头录像
//参数1 摄像头序列号
//参数2 录像标志   true:开始\false:停止
//参数3 MP4保存路径 
//参数4 输出错误码 
void cameraRecod(char* szCameraSerialNumber, bool recodFlag, char* szMP4Path, int* iErrorCode)
{
	*iErrorCode = 0;
	/*LONG lUserID = 0; LONG lChannel = 0; LONG lPlayHandle = 0;*/ int index_ = 0;
	if (getSNmapInfo(szCameraSerialNumber,/* &lUserID, &lChannel, &lPlayHandle,*/ &index_)) //判断序列号是否在列表中
	{
		//判断路径的文件夹是否存在
		detectPath(szMP4Path); 

		if (recodFlag)  //启动录像
		{
			if (vtCamera[index_].m_bIsRecording == false){
				if (vtCamera[index_].m_bLogin == false){
					 loginCamera(szCameraSerialNumber, iErrorCode);   //先登录
				}
				if (vtCamera[index_].m_bLogin == true ){  
					if (!NET_DVR_SaveRealData(vtCamera[index_].m_lPlayHandle, szMP4Path))
					{
						printf("启动录像失败，错误代码%d\n", NET_DVR_GetLastError());
						*iErrorCode = NET_DVR_GetLastError();
						logInfo(errorLog, "启动录像失败，错误代码%d", NET_DVR_GetLastError());
						logoutCamera(szCameraSerialNumber, iErrorCode);
					}
					else{
						printf("启动录像成功，路径在%s， 观看前请停止录像。\n", szMP4Path);
						vtCamera[index_].m_bIsRecording = true;
					}
				}

			}
			else{
				printf("%s正在录像\n", szCameraSerialNumber);
				*iErrorCode = E_RECODING;
			} 
		}
		else  //停止录像
		{
			if (vtCamera[index_].m_bIsRecording == true){
				if (!NET_DVR_StopSaveRealData(vtCamera[index_].m_lPlayHandle))
				{
					printf("停止录像失败，错误代码%d\n", NET_DVR_GetLastError());
					*iErrorCode = NET_DVR_GetLastError();
				}
				else{
					printf("停止录像成功.\n");
					vtCamera[index_].m_bIsRecording = false;
					if (vtCamera[index_].m_bLogin == true){
						logoutCamera(szCameraSerialNumber, iErrorCode);   //登出
					}
				}
			}
			else
			{
				printf("%s没有启动录像\n", szCameraSerialNumber);
				*iErrorCode = E_NOT_RECODING;
			}
		}
	}
	else{
		*iErrorCode = E_NOT_SN;
	}
}



LRESULT CALLBACK WinSunProc(
	HWND hwnd,  // 窗口的句柄
	UINT uMsg,   //消息标识符
	WPARAM wParam, // 第一个消息参数
	LPARAM lParam  //第二个消息参数
	);  //回调函数

DWORD WINAPI ThreadProFunc(LPVOID lpParam);

//创建一个播放窗口
HWND CreatePlayWindow(const TCHAR* szClassName, int* errorCode) 
{
	*errorCode = 0;
	WNDCLASS wndcls;    //实例化
	wndcls.cbClsExtra = 0;   //额外的类的附加字节数
	wndcls.cbWndExtra = 0;   //窗口额外的附加字节数
	//HGDIOBJ GetStockObject(int fnObject);检索预定义的备用笔、刷子、字体或者调色板的句柄
	//BLACK_BRUSH 指黑色画刷
	wndcls.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);  //背景颜色，GetStockObject()函数的返回值是一个HGDIOBJ格式的，强制转换成HBRUSH格式的赋值
	//LoadCursor载入指定的光标类型
	//原形：HCURSOR LoadCursor(HINSTANCE hlnstance，LPCTSTR lpCursorName);
	//hlnstance标识一个模块事例，它的可执行文件包含要载入的光标,lpCursorName光标资源名
	wndcls.hCursor = LoadCursor(NULL, IDC_CROSS);  //光标的类型 IDC_CROSS代表十字光标
	// LoadIcon函数已经被LoadImage替代
	wndcls.hIcon = LoadIcon(NULL, IDI_ERROR);   //图标
	wndcls.hInstance = NULL;//hInstance;              //应用程序的实例号
	wndcls.lpfnWndProc = WinSunProc;       //窗口过程函数
	wndcls.lpszClassName = szClassName;  //类名
	wndcls.lpszMenuName = NULL;   //菜单名
	wndcls.style = CS_HREDRAW | CS_VREDRAW; //水平重画和垂直重画 

	RegisterClass(&wndcls);   //注册窗口类 

	static int s_timer_x = 0; static int s_timer_y = 0;

	HWND hWnd = CreateWindow(szClassName, szClassName, WS_OVERLAPPEDWINDOW, (s_timer_x % 3 * 400), (s_timer_y / 3 * 213), 600, 320, NULL, NULL, NULL, NULL); //建立一个窗口
	ShowWindow(hWnd, SW_SHOWNORMAL);   //显示或者隐藏窗口  
	UpdateWindow(hWnd);  //更新窗口
	s_timer_x++; s_timer_y++;

	static bool first_ = true;
	if (first_){
		hThread = CreateThread(NULL	// 默认安全属性
			, NULL		// 默认堆栈大小
			, ThreadProFunc // 线程入口地址
			, NULL	//传递给线程函数的参数
			, 0		// 指定线程立即运行
			, &dwThreadId	//线程ID号
			);
		first_ = false;
	}

	return hWnd; 
}

DWORD WINAPI ThreadProFunc(LPVOID lpParam)
{
	MSG msg;   //消息机制
	while (GetMessage(&msg, NULL, 0, 0))    //消息循环
	{
		TranslateMessage(&msg);   //将传来的消息翻译
		DispatchMessage(&msg);    //
	}
	return 0;
}

LRESULT CALLBACK WinSunProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		//if (IDYES == MessageBox(hwnd, L"是否退出？", L"tips", MB_YESNO))
	{
		DestroyWindow(hwnd);  //销毁窗口
	}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		CloseHandle(hThread);	//关闭线程句柄，内核引用计数减一
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

void getPlayHandle(stCamera& vtCameraObj, int iChanIndex)  //获取播放通道
{
	NET_DVR_CLIENTINFO ClientInfo;
	ClientInfo.hPlayWnd = vtCameraObj.m_hWnd;//这里需要一个窗口句柄 
	ClientInfo.lChannel = iChanIndex ;
	ClientInfo.lLinkMode = 0;
	ClientInfo.sMultiCastIP = NULL;
	vtCameraObj.m_lPlayHandle = NET_DVR_RealPlay_V30(vtCameraObj.m_struDeviceInfo.lLoginID, &ClientInfo, NULL, NULL, TRUE);
	if (-1 == vtCameraObj.m_lPlayHandle)
	{
		DWORD err = NET_DVR_GetLastError();
		printf("播放出错，错误代码%d\n", err);
		logInfo(errorLog, "播放出错，错误代码%d", err);
		return;
	}

	vtCameraObj.m_bIsPlaying = true;
}

/*多字节转宽字节*/
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


/*宽字节转多字节*/
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


//判断序列号是否在vector中
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

//分割字符
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

//写入log信息
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
			fopen_s(&fpError,szTemp, "ab+");/*以读/写方式打开一个二进制文件，允许读或在文件末追加数据。如果不存在,会自动创建*/
		}
		if (fpError){
			fwrite(szLogSumb, strlen(szLogSumb), 1, fpError);
		}
	}
}

//关闭log文件具备
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
	if (!vtCameraObj.m_struDeviceInfo.bIPRet)   //不支持ip接入,9000以下设备不支持禁用模拟通道
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)
		{
			if (i < vtCameraObj.m_struDeviceInfo.iDeviceChanNum)
			{
				sprintf_s(vtCameraObj.m_struDeviceInfo.struChanInfo[i].chChanName, "camera%d", i + vtCameraObj.m_struDeviceInfo.iStartChan);
				vtCameraObj.m_struDeviceInfo.struChanInfo[i].iChanIndex = i + vtCameraObj.m_struDeviceInfo.iStartChan;  //通道号
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
	else        //支持IP接入，9000设备
	{
		for (i = 0; i < MAX_ANALOG_CHANNUM; i++)  //模拟通道
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

		//数字通道
		for (i = 0; i < MAX_IP_CHANNEL; i++)
		{
			if (IpAccessCfg.struStreamMode[i].uGetStream.struChanInfo.byEnable)  //ip通道在线
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

//探索文件路径，如果不存在路径，则创建
void detectPath(const char* path_)  
{
	std::string strPath = path_;
	int pos = strPath.find_last_of('\\', strPath.length());
	if (pos == -1)
		return;
	strPath = strPath.substr(0, pos); 
	detectPath(strPath.c_str());
	if (_access(strPath.c_str(), 0) == -1){ //判断文件是否存在
		CreateDirectoryA(strPath.c_str(), NULL);
	}

}