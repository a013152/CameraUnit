
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "parJSON.h"


msgFieldDef regFieldArrary[F_NUM_R] = {
	{ F_SERIAL_R, "serialNum" },
	{ F_DEVICEID_R, "deviceID" },
	{ F_ACTION_R, "action" },
	{ F_DURATION_R, "duration" }
};

/******************************************************************************
* 自定义根据键获取值
* ****************************************************************************/
int jsonGetValue(cJSON *in_json, char *in_key, char **out_value)
{
	cJSON *dataObj;

	if (in_key == NULL)
	{
		return E_FAILD;
	}

	if (in_json == NULL)
	{
		return E_NOT_JSON;
	}

	if (1 != cJSON_HasObjectItem(in_json, in_key))
	{
		return E_GET_VALUE;
	}
	else
	{
		dataObj = cJSON_GetObjectItem(in_json, in_key);

		if (dataObj->type == cJSON_String)
		{
			  
			*out_value = malloc(strlen(dataObj->valuestring) + 1);
			memset(*out_value, 0, strlen(dataObj->valuestring) + 1);

			if (*out_value == NULL)
			{
				return E_GET_VALUE;
			}

			memcpy(*out_value, dataObj->valuestring, strlen(dataObj->valuestring));
			//strcpy_s(*out_value, 3, dataObj->valuestring);

			return 0;
		}
		else
		{
			return E_GET_VALUE;
		}
	}
}

int isDigitStr(char *str)
{
	char *p = str;

	while (*p)
	{
		if ('0' > *p || '9' < *p)
			return 0;

		p++;
	}

	return 1;
}

int parJson(cJSON *in_json, char **out_str)
{
	int iRet = -1;
	//cJSON *json = cJSON_Parse(in_str);
	int i = 0;

	while (i < F_NUM_R)
	{
		iRet = jsonGetValue(in_json, regFieldArrary[i].fieldKey, &out_str[i]);
		printf("%s -> %s\n", regFieldArrary[i].fieldKey, out_str[i]);

		//安全检测
		switch (i)
		{
			case F_DURATION_R:
				if (iRet < 0)
					iRet = E_SUCCESS;		//允许为空，默认录像十秒钟
				else if (!isDigitStr(out_str[i]))
					iRet = E_ILLEGAL_V;
				break;
			case F_ACTION_R:
				if (iRet == E_SUCCESS )
				{
					if (!isDigitStr(out_str[i]))
						iRet = E_ILLEGAL_V;
					else if (atoi(out_str[i]) != R_CAM_CAP && atoi(out_str[i]) != R_CAM_REC)
						iRet = E_UNHAND_REQ;
				}
					
				break;
			default:
				break;
		}

		if (iRet < 0)
			return iRet;
		i++;
	}

	return 0;
}
