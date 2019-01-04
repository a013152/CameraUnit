
#ifndef _PAR_JSON__H
#define _PAR_JSON__H

#include "cJSON.h"


#define E_SUCCESS		0
#define E_FAILD			-1
#define E_NOT_JSON		-2		//请求报文不是json格式
#define E_GET_VALUE		-3		//字段获取值失败
#define E_UNHAND_REQ	-4		//未知的处理类型请求，即不是截图和录像
#define E_ILLEGAL_V		-5		//值的类型为非法字符

typedef enum _reqMsgField
{
	F_SERIAL_R,
	F_DEVICEID_R,
	F_ACTION_R,
	F_DURATION_R,
	F_NUM_R
}regMsgField;

enum _MSG_REQ_TYPE_{
	R_CAM_CAP = 1,			//截图
	R_CAM_REC				//录像
};

typedef struct _msgFieldDef
{
	int  fieldId;
	char fieldKey[30];
} msgFieldDef;

int parJson(cJSON *in_json, char **out_str);

#endif