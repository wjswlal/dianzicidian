#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "msg.h"

#define SA struct sockaddr


int do_regist(int sockfd,msg_t *msg)
{
	msg->type=RIG;
	printf("请输入帐号：\n");
	scanf("%s",msg->buf1);
	printf("请输入密码，区分大小写：\n");
	scanf("%s",msg->buf2);
	send(sockfd,msg,sizeof(msg_t),0);

	bzero(msg,sizeof(msg_t));
	recv(sockfd,msg,sizeof(msg_t),0);
	if(msg->type==RIG_OK)
	{
		printf("注册成功,请退出重新登录\n");
	}
	else
	{
		printf("注册失败，该帐号已存在，请重新注册\n");
		return -1;
	}

}

int do_word(int sockfd,msg_t *msg)
{

	while(1)
	{
		printf("请输入word:");
		bzero(msg->buf1,sizeof(msg->buf1));
		msg->type=QUR;
		fgets(msg->buf1,sizeof(msg->buf1),stdin);
		if(msg->buf1[strlen(msg->buf1)-1]=='\n')
			msg->buf1[strlen(msg->buf1)-1]='\0';
		if(strncmp(msg->buf1,"666",3)==0)
			break;

		send(sockfd,msg,sizeof(msg_t),0);

		bzero(msg,sizeof(msg_t));
		recv(sockfd,msg,sizeof(msg_t),0);
		if(msg->type==QUR_OK)
		{
			printf("%s\n",msg->buf2);
		}
		else
		{
			printf("not find!\n");
		}
	}
}

int do_history(int sockfd,msg_t *msg)
{
	msg->type=HIT;
	send(sockfd,msg,sizeof(msg_t),0);

	while(1)
	{
//	bzero(msg->buf2,sizeof(msg->buf2));
	recv(sockfd,msg,sizeof(msg_t),0);
	if(msg->buf2[0]=='\0')
		break;
	printf("%s\n",msg->buf2);
	}
	return 0;
}
int do_login(int sockfd,msg_t *msg)
{

	bzero(msg,sizeof(msg_t));
	msg->type=LOG;
	printf("请输入帐号：");
	fgets(msg->buf1,sizeof(msg->buf1),stdin);
	if(msg->buf1[strlen(msg->buf1)-1]=='\n')
		msg->buf1[strlen(msg->buf1)-1]='\0';

	printf("请输入密码：");
	fgets(msg->buf2,sizeof(msg->buf2),stdin);
	if(msg->buf2[strlen(msg->buf2)-1]=='\n')
		msg->buf2[strlen(msg->buf2)-1]='\0';

	send(sockfd,msg,sizeof(msg_t),0);

	bzero(msg,sizeof(msg_t));
	recv(sockfd,msg,sizeof(msg_t),0);
	if(msg->type==LOG_OK)
	{
		printf("登录成功！\n");
		return 1;
	}
	else 
	{
		printf("登录失败！请重新登录！\n");
		return -1;
	}
}
int do_login_success(int sockfd,msg_t *msg)
{
	while(1)
	{
		int b=0;
		printf("1:查单词 2：查历史记录 3：退出\n");
		printf("请输入你要选择的命令对应的数字:\n");
		scanf("%d",&b);
		getchar();
		switch(b)
		{
		case 1:
			do_word(sockfd,msg);
			break;
		case 2:
			do_history(sockfd,msg);
			break;
		case 3:
			close(sockfd);
			exit(0);
			break;
		default:
			printf("输入有误\n");
			break;


		}
	}
}


int main(int argc, const char *argv[])
{

	if(argc!=3)
	{
		printf("please to input\n");
		exit(1);
	}


	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(socket<0)
	{
		perror("fail to socket");
		exit(1);
	}
	struct sockaddr_in myaddr,peeraddr;
	bzero(&myaddr,sizeof(myaddr));
	myaddr.sin_family=AF_INET;
	myaddr.sin_port=htons(50000);
	myaddr.sin_addr.s_addr=inet_addr("0.0.0.0");
	
	bzero(&peeraddr,sizeof(peeraddr));
	peeraddr.sin_family=AF_INET;
	peeraddr.sin_port=htons(atoi(argv[2]));
	peeraddr.sin_addr.s_addr=inet_addr(argv[1]);


	int ret_connect=connect(sockfd,(SA*)&peeraddr,sizeof(peeraddr));
	if(ret_connect<0)
	{
		perror("fail to connect");
		exit(1);
	}

	msg_t *msg=malloc(sizeof(msg_t));
/*	char buf[128];
	char id[100];
	char pw[100];*/
	int a=0;
		while(1)
		{
			printf("1：注册 2：登录 3：退出\n");
			printf("请输入你要选择的命令对应的数字:\n");
			
			scanf("%d",&a);
			getchar();
			switch(a)
			{
				case 1://注册	
					do_regist(sockfd,msg);	
					break;
				case 2://登录
					if(do_login(sockfd,msg)==1)
					do_login_success(sockfd,msg);
					break;
				case 3://退出
					close(sockfd);
					exit(0);
					break;
				default:
					printf("输入有误\n");
					break;

			
			}

		}
		
	
	return 0;
}
	
