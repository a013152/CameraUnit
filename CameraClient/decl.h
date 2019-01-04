#include "stdafx.h"
#include "GeneralDef.h" 
#include <vector>
#include <time.h>
    
extern int iErrorCode;//错误码
extern LONG lLoginID;
extern char g_AppPath[256]; 
extern char szCapturePath[256] ;  //截图保存路径
extern char szRecordePath[256] ;	//录像保存路径

typedef struct stCamera_{
	LOCAL_DEVICE_INFO m_struDeviceInfo;  //本地设备信息
	NET_DVR_DEVICEINFO_V30 DeviceInfo;  //设备信息
	bool m_bLogin;		//登录标志
	bool m_bIsPlaying;		//正在播放标志
	bool m_bIsRecording ;	//正在录像标志
	HWND m_hWnd ;	// 播放窗口句柄
	LONG m_lPlayHandle; //当前通道
	std::string strSerialNumber;
	char szIP[256];
	char szUSER[256];
	char szPASSWORD[256];
	unsigned int nPort;
	stCamera_() :m_bLogin(false),m_bIsPlaying(false), m_bIsRecording(false), m_hWnd(NULL), m_lPlayHandle(-1), nPort(8000){
		memset(&m_struDeviceInfo, 0, sizeof(LOCAL_DEVICE_INFO));
		memset(&DeviceInfo, 0, sizeof(NET_DVR_DEVICEINFO_V30));
		memset(szIP, 0, 256);
		memset(szUSER, 0, 256);
		memset(szPASSWORD, 0, 256);
	}
}stCamera;

std::vector<stCamera>& getVTCamera();



enum ERROR_CODE_{
	E_INI_HKSDK = 20001,         //海康SDK初始化错误 ，排除系统问题，否则无解。
	E_INI_FILE_NOT_EXIST ,	//ini配置不存在	
	E_INI_FILE_CAMER_NUMBER,	//摄像头数量错误
	E_INI_FILE_CAMERA_IP,		//摄像头ip错误
	E_LOGIN_CAMERA,				//登录摄像头错误
	E_NOT_SN,			//没找到SN
	E_RECODING,			//正在录像
	E_NOT_RECODING			//没有启动录像
};

//初始化 
bool initCamera(int* iErrorcode);

//反初始化
void uninitCamera();



//读取配置，获得需要登录的摄像头信息、路径信息等
void readConfig(int *iErrorCode);  

//登录摄像头， 
//参数1 摄像头序列号
//参数2 输出错误码 
bool loginCamera(char* szCameraSerialNumber ,int* iErrorcode);

//登出摄像头， 
//参数1 摄像头序列号
//参数2 输出错误码 
bool logoutCamera(char* szCameraSerialNumber, int* iErrorcode);

//摄像头拍照
//参数1 摄像头序列号
//参数2 图片保存路径
//参数3 输出错误码 
void cameraCapture(char* szCameraSerialNumber, char* szJpgPath,int* iErrorcode);

//摄像头录像
//参数1 摄像头序列号
//参数2 录像标志   true:开始\false:停止
//参数3 MP4保存路径 
//参数4 输出错误码 
void cameraRecod(char* szCameraSerialNumber, bool recodFlag, char* szMP4Path, int* iErrorcode);



/*获取序列号对应的 通道信息，未找到返回false*/
bool getSNmapInfo(char* szCameraSerialNumber,/* LONG* lUserID, LONG* lChannel, LONG* lPlayHandle,*/ int *index);

/*多字节转宽字节*/
static std::wstring MBytesToWString(const char* lpcszString);

/*宽字节转多字节*/
static std::string WStringToMBytes(const wchar_t* lpwcszWString);

/*分割字符串*/
void splitString(std::string strtem, char a, std::vector<std::string>& vtStrCommand);

//写入log信息
enum logType{
	errorLog 
};
void logInfo(logType, const char* format, ...); 

void closeLog();

void DoGetDeviceResoureCfg(stCamera& );

HWND CreatePlayWindow(const TCHAR* szClassName, int* errorCode); //创建播放窗口

void getPlayHandle(stCamera&, int);  //获取播放通道

void detectPath(const char* path_);  //探索文件路径，如果不存在路径，则创建





