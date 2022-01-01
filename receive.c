#include<stdio.h>
#include"sem.h"
#include"box.h"


int main()
{
	struct box* boxA = getNewBox(1, boxNum, boxLen);
	struct box* boxB = getNewBox(2, boxNum, boxLen);
	while(1)
	{
		printf("请输入你要执行的操作:");
		char input[256];
		memset(input, 0, 256);
		scanf("%s", input);
		if(strncmp(input, "send",4) == 0)
		{
			printf("请输入要发送的信件：");
			char msg[20];
			scanf("%s",&msg);
			send(boxA, msg, strlen(msg));
		}
		else if(strncmp(input, "receive",7) == 0 )
		{
			char msg[20];
			receive(boxB, msg);
			printf("有一封邮件请查收！内容是：%s\n", msg);
		}
		else if(strncmp(input, "withdraw",8) == 0)
		{
			recall(boxA);
		}
		else if(strncmp(input, "delete",6) == 0)
		{
			deleteBox(boxB);
			break;
		}
		else
		{
			printf("无效操作，请再次尝试！!\n");
		}
	}
	return 0;
}
