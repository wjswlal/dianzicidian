#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>	
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include "msg.h"
#include <string.h>
#include <sqlite3.h>

#define SA struct sockaddr
int count=0,denex=0,i=0;
//int do_regist(int confd,msg_t *msg,sqlite3 *db)

int callback(void *chuancan,int n,char **colval, char **colname)
{
	count++;
	return 0;
}
int callback2(void *chuancan,int n,char **colval, char **colname)
{
	denex++;
	strcpy((char *)chuancan,colval[0]);
	return 0;
}

int callback3(void *chuancan,int n,char **colval, char **colname)
{
//	strcat((char *)chuancan,colval[0]);
	i++;
	int confd=*(int *)chuancan;
	msg_t msg;
	sprintf(msg.buf2,"%s",colval[0]);
	if(i>=20)
	msg.buf2[0]='\0';
	send(confd,&msg,sizeof(msg),0);
	printf("**************\n");
	return 0;

}
int do_regist(int confd,msg_t *msg,sqlite3 *db)
{
	char sql[256],*errmsg;
	bzero(sql,sizeof(sql));
	printf("%s\n",msg->buf1);
	sprintf(sql,"select id from information	where id=='%s'",msg->buf1);
	int temp=count;
	int ret_sql=sqlite3_exec(db,sql,callback,NULL,&errmsg);
	if(ret_sql)
	{
		fprintf(stderr,"fail to find:%s\n",errmsg);
		sqlite3_free(errmsg);
		exit(1);
	}

	if(count==temp)//不存在用户名，注册成功
	{

		msg->type=RIG_OK;	
		sprintf(sql,"insert into information values('%s','%s')",
				msg->buf1,msg->buf2);
		ret_sql=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
		if(ret_sql)
		{
			fprintf(stderr,"fail to exec:%s\n",errmsg);
			sqlite3_free(errmsg);
		}
		send(confd,msg,sizeof(msg_t),0);
	}
	else//用户名存在，注册失败
	{
		msg->type=RIG_NOK;
		send(confd,msg,sizeof(msg_t),0);
	}
}
int do_login(int confd,msg_t *msg,sqlite3 *db)
{

	char sql[256],*errmsg;
	
	bzero(sql,sizeof(sql));
	sprintf(sql,"select id from information	where id=='%s' and pw=='%s'",msg->buf1,msg->buf2);

	int temp1=count;
	//	printf("%d\n",count);
	int ret_sql=sqlite3_exec(db,sql,callback,NULL,&errmsg);
	if(ret_sql)
	{
		fprintf(stderr,"fail to find:%s\n",errmsg);
		sqlite3_free(errmsg);
		exit(1);
	}
	if(temp1==count)
	{
		msg->type=LOG_NOK;
		send(confd,msg,sizeof(msg_t),0);
	}
	else 
	{
		strcpy(msg->buf3,msg->buf1);
		msg->type=LOG_OK;
		send(confd,msg,sizeof(msg_t),0);
	}

}
int do_word(int confd,msg_t *msg,sqlite3 *db)
{
	char sql[256],*errmsg;

	bzero(sql,sizeof(sql));
	sprintf(sql,"insert into history values('%s','%s')",msg->buf3,msg->buf1);
	int ret_sql=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
	if(ret_sql)
	{
		fprintf(stderr,"fail to exec:%s\n",errmsg);
		sqlite3_free(errmsg);
	}


	sprintf(sql,"select des from dict where word=='%s'",msg->buf1);
	int temp2=denex;
	ret_sql=sqlite3_exec(db,sql,callback2,msg->buf2,&errmsg);
	if(ret_sql)
	{
		fprintf(stderr,"fail to find:%s\n",errmsg);
		sqlite3_free(errmsg);
		exit(1);
	}

	if(temp2==denex)
	{
		msg->type=QUR_NOK;
	}
	else 
	{
		msg->type=QUR_OK;
	}
	send(confd,msg,sizeof(msg_t),0);
}

int do_history(int confd,msg_t *msg,sqlite3 *db)
{
	char sql[512],*errmsg;
//	bzero(msg->buf2,sizeof(msg->buf2));
	bzero(sql,sizeof(sql));
	sprintf(sql,"select word from history where id=='%s'",msg->buf3);
	int ret_sql=sqlite3_exec(db,sql,callback3,(void *)&confd,&errmsg);
	if(ret_sql)
	{
		fprintf(stderr,"fail to find:%s\n",errmsg);
		sqlite3_free(errmsg);
		exit(1);
	}
	msg->buf2[0]='\0';
	send(confd,msg,sizeof(msg_t),0);
}



int main(int argc, const char *argv[])
{

	sqlite3 *db=NULL;
	char *errmsg=NULL;

	char sql[512];

	int ret_sql;
	ret_sql=sqlite3_open("./dic.db",&db);
	if(ret_sql)
	{
		perror("fail to sqlite3_open");
		exit(1);
	}

	int sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(socket<0)
	{
		perror("fail to socket");
		exit(1);
	}
	pid_t pid;
	signal(SIGCHLD,SIG_IGN);

	struct sockaddr_in myaddr,peeraddr;
	bzero(&myaddr,sizeof(myaddr));
	myaddr.sin_family=AF_INET;
	myaddr.sin_port=htons(58888);
	myaddr.sin_addr.s_addr=inet_addr("0.0.0.0");
	
	int ret_bind=bind(sockfd,(SA*)&myaddr,sizeof(myaddr));
	if(ret_bind<0)
	{
		perror("fail to bind");
		exit(1);
	}

	int ret_listen=listen(sockfd,5);
	if(ret_listen<0)
	{
		perror("fail to listen");
		exit(1);
	}

	int rec_recv;
	socklen_t len=sizeof(peeraddr);

	msg_t *msg=malloc(sizeof(msg_t));
//	char buf[128];
	while(1)
	{
		bzero(&peeraddr,sizeof(peeraddr));

		int confd=accept(sockfd,(SA*)&peeraddr,&len);
		if(confd<0)
		{
			perror("fail to accept");
			exit(1);
		}
		printf("%s is connect  %d is creat\n",inet_ntoa(peeraddr.sin_addr),confd);

		pid=fork();
		if(pid<0)
		{
			perror("fail to pid");
			exit(1);
		}
		else if(pid==0)
		{
			close(sockfd);

			while(1)
			{
				bzero(msg,sizeof(msg_t));
				rec_recv=recv(confd,msg,sizeof(msg_t),0);
				if(rec_recv<0)
				{
					perror("fail to recv");
					exit(1);
				}
				else if(rec_recv==0)
				{
					printf("perr is shoudown\n");
					exit(0);
				}
				else
				{
					switch(msg->type)
					{
					case RIG://注册
						do_regist(confd,msg,db);
					break;

					case LOG://登录
						do_login(confd,msg,db);
					
						break;
					case QUR://查单词
						do_word(confd,msg,db);
			
						break;
					case HIT://查历史记录
						do_history(confd,msg,db);
						break;
					default:
						;
					}
				}

			}
		}
		else
		{
			close(confd);
		}
	
		
	}



	return 0;
}
