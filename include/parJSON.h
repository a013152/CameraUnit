
#ifndef _PAR_JSON__H
#define _PAR_JSON__H

#include "cJSON.h"


#define E_SUCCESS		0
#define E_FAILD			-1
#define E_NOT_JSON		-2		//�����Ĳ���json��ʽ
#define E_GET_VALUE		-3		//�ֶλ�ȡֵʧ��
#define E_UNHAND_REQ	-4		//δ֪�Ĵ����������󣬼����ǽ�ͼ��¼��
#define E_ILLEGAL_V		-5		//ֵ������Ϊ�Ƿ��ַ�

typedef enum _reqMsgField
{
	F_SERIAL_R,
	F_DEVICEID_R,
	F_ACTION_R,
	F_DURATION_R,
	F_NUM_R
}regMsgField;

enum _MSG_REQ_TYPE_{
	R_CAM_CAP = 1,			//��ͼ
	R_CAM_REC				//¼��
};

typedef struct _msgFieldDef
{
	int  fieldId;
	char fieldKey[30];
} msgFieldDef;

int parJson(cJSON *in_json, char **out_str);

#endif