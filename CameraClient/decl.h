#include "stdafx.h"
#include "GeneralDef.h" 
#include <vector>
#include <time.h>
    
extern int iErrorCode;//������
extern LONG lLoginID;
extern char g_AppPath[256]; 
extern char szCapturePath[256] ;  //��ͼ����·��
extern char szRecordePath[256] ;	//¼�񱣴�·��

typedef struct stCamera_{
	LOCAL_DEVICE_INFO m_struDeviceInfo;  //�����豸��Ϣ
	NET_DVR_DEVICEINFO_V30 DeviceInfo;  //�豸��Ϣ
	bool m_bLogin;		//��¼��־
	bool m_bIsPlaying;		//���ڲ��ű�־
	bool m_bIsRecording ;	//����¼���־
	HWND m_hWnd ;	// ���Ŵ��ھ��
	LONG m_lPlayHandle; //��ǰͨ��
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
	E_INI_HKSDK = 20001,         //����SDK��ʼ������ ���ų�ϵͳ���⣬�����޽⡣
	E_INI_FILE_NOT_EXIST ,	//ini���ò�����	
	E_INI_FILE_CAMER_NUMBER,	//����ͷ��������
	E_INI_FILE_CAMERA_IP,		//����ͷip����
	E_LOGIN_CAMERA,				//��¼����ͷ����
	E_NOT_SN,			//û�ҵ�SN
	E_RECODING,			//����¼��
	E_NOT_RECODING			//û������¼��
};

//��ʼ�� 
bool initCamera(int* iErrorcode);

//����ʼ��
void uninitCamera();



//��ȡ���ã������Ҫ��¼������ͷ��Ϣ��·����Ϣ��
void readConfig(int *iErrorCode);  

//��¼����ͷ�� 
//����1 ����ͷ���к�
//����2 ��������� 
bool loginCamera(char* szCameraSerialNumber ,int* iErrorcode);

//�ǳ�����ͷ�� 
//����1 ����ͷ���к�
//����2 ��������� 
bool logoutCamera(char* szCameraSerialNumber, int* iErrorcode);

//����ͷ����
//����1 ����ͷ���к�
//����2 ͼƬ����·��
//����3 ��������� 
void cameraCapture(char* szCameraSerialNumber, char* szJpgPath,int* iErrorcode);

//����ͷ¼��
//����1 ����ͷ���к�
//����2 ¼���־   true:��ʼ\false:ֹͣ
//����3 MP4����·�� 
//����4 ��������� 
void cameraRecod(char* szCameraSerialNumber, bool recodFlag, char* szMP4Path, int* iErrorcode);



/*��ȡ���кŶ�Ӧ�� ͨ����Ϣ��δ�ҵ�����false*/
bool getSNmapInfo(char* szCameraSerialNumber,/* LONG* lUserID, LONG* lChannel, LONG* lPlayHandle,*/ int *index);

/*���ֽ�ת���ֽ�*/
static std::wstring MBytesToWString(const char* lpcszString);

/*���ֽ�ת���ֽ�*/
static std::string WStringToMBytes(const wchar_t* lpwcszWString);

/*�ָ��ַ���*/
void splitString(std::string strtem, char a, std::vector<std::string>& vtStrCommand);

//д��log��Ϣ
enum logType{
	errorLog 
};
void logInfo(logType, const char* format, ...); 

void closeLog();

void DoGetDeviceResoureCfg(stCamera& );

HWND CreatePlayWindow(const TCHAR* szClassName, int* errorCode); //�������Ŵ���

void getPlayHandle(stCamera&, int);  //��ȡ����ͨ��

void detectPath(const char* path_);  //̽���ļ�·�������������·�����򴴽�





