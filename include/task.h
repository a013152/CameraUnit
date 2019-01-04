#include "ThreadPool.h"

#define PHOTO_GRAPH		1
#define VIDEO_TAPE		2	

class Task
{
public:
	static int parMsg(PVOID pMsgStr);
	static int Task1(PVOID p)
	{ 
		printf("Task::%d\n", (int)p);
		Sleep(100);

		return (int)p;
	}
};

typedef struct parmStructDef
{
	SOCKET sockets;
	char *msgBuf;
}parmStructDef;



class TaskCallback
{
public:
	static void TaskCallback1(int result)
	{
		printf("TaskCB::   %d\n", result);
	}
};
