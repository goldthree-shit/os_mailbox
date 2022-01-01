#ifndef box_h
#define box_h

#include"sem.h"
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<sys/shm.h>

#define boxId  1234
#define boxNum 5 // 消息数量
#define boxLen 5 // 单条消息长度

struct box
{
//信箱头
	int bid;//信箱标识符
	int bnum;//信格总数
	int blen; //单个信格的大小
	/*同步信号量*/
	int mailnum;//与信箱中信件数量相关的信号量
	int freenum;//与信箱中空信格数量相关的信号量
	/*互斥信号量*/
	int rmutex;//接收信件时的互斥信号量
	int wmutex;//存入信件时的互斥信号量
//信箱体
	char  *buf; // 消息数组
	int   *buf_len; // 每一条消息的长度数组
	void  *shm1;
	void  *shm2;
	// 注意 in && out 不是共享的，只有 buf 是共享的
	// in && out 指向buf_len
	int out;//当前可读取信件的信格地址
	int in;//当前可存入信件的信格地址
};

struct box* getNewBox(int n, int num, int len)//获取新信箱, n为唯一邮箱变量,num为信格总数，len为单个信格大小
{
	struct box* msgbox = (struct box*)malloc(sizeof(struct box));
	int shmid1 = shmget((key_t)(boxId+n),sizeof(char)*num*len,0666|IPC_CREAT);
	int shmid2 = shmget((key_t)(boxId+n+5),sizeof(int)*num,0666|IPC_CREAT);
	if(shmid1 == -1 || shmid2 == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }
	msgbox->shm1 = shmat(shmid1,0,0);
	msgbox->shm2 = shmat(shmid2,0,0);
	msgbox->buf = (char *)(msgbox->shm1);
	msgbox->buf_len = (int *)(msgbox->shm2);
	//初始化信格总数
	msgbox->bnum = num;
	// 初始化每个信格大小
	msgbox->blen = len;
	//初始化邮箱标号，为共享信号的标号
	msgbox->bid = shmid1;
	//初始化信号量
	msgbox->mailnum = getNewSem((key_t)(shmid1+1));
	setSemValue(msgbox->mailnum,0);

	msgbox->freenum = getNewSem((key_t)(shmid1+2));
	setSemValue(msgbox->freenum,len);

	msgbox->rmutex = getNewSem((key_t)(shmid1+3));
	setSemValue(msgbox->rmutex,1);

	msgbox->wmutex = getNewSem((key_t)(shmid1+4));
	setSemValue(msgbox->wmutex,1);

	msgbox->out = 0;
	msgbox->in = 0;

	return msgbox;
}

void send(struct box* dest,char *msg, int msg_len)//发送
{
	P(dest->freenum);
	P(dest->wmutex);
	// dest->buf[dest->in] = msg;
	if((strlen(dest->buf) + msg_len) > dest->blen * dest->bnum || (dest->in + 1 > dest->bnum)) 
	{
		printf("error 超过信箱大小!\n");
		return ;
	}
	printf("发送字符串 %s\n", msg);
	// 当 src 的长度小于 n 时，dest 的剩余部分将用空字节填充。
	// 保证每个信格长度一致，都是 dest->blen
	strncpy(dest->buf + dest->in * dest->blen, msg, dest->blen);
	dest->buf_len[dest->in] = msg_len;
	dest->in ++;
	V(dest->wmutex);
	V(dest->mailnum);
};


char receive(struct box* addr, char *dest)//接收
{
	//int msg;
	P(addr->mailnum);
	P(addr->rmutex);
	// msg = addr->buf[addr->out];
	int size = addr->buf_len[addr->out];
	strncpy(dest, addr->buf + addr->out * addr->blen, size);
	addr->out ++;
	V(addr->rmutex);
	V(addr->freenum);
};

void recall(struct box* addr)//撤销
{
	P(addr->mailnum);
	P(addr->wmutex);
	P(addr->rmutex);
	addr->in --;
	V(addr->rmutex);
	V(addr->wmutex);
	V(addr->freenum);
};
void deleteBox(struct box* msgbox)//删除信箱
{
	delSem(msgbox->freenum);
	delSem(msgbox->mailnum);
	delSem(msgbox->rmutex);
	delSem(msgbox->wmutex);
	shmdt(msgbox->shm1);
	shmdt(msgbox->shm2);
	shmctl(msgbox->bid,IPC_RMID,0);
};


#endif
