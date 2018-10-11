#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define  N  16
#define  R  1   //  user register
#define  L  2   //  user login
#define  Q  3   //  query word
#define  H  4   //  history record
#define  COL 50

#define DATABASE "my.db"

typedef struct 
{
	int type;
	char name[N];
	char data[256];   // password or word or remark
} MSG;

void do_register(int socketfd, MSG *msg)
{
	msg->type = R;
	printf("input name : ");
	scanf("%s", msg->name);
	printf("input password : ");
	scanf("%s", msg->data);
	send(socketfd, msg, sizeof(MSG), 0);
	recv(socketfd, msg, sizeof(MSG), 0);
	printf("register : %s\n", msg->data);

	return;
}

int do_login(int socketfd, MSG *msg)
{
	msg->type = L;
	printf("input name : ");
	scanf("%s", msg->name);
	printf("input password : ");
	scanf("%s", msg->data);
	send(socketfd, msg, sizeof(MSG), 0);
	recv(socketfd, msg, sizeof(MSG), 0);
	if (strncmp(msg->data, "OK", 3) == 0) 
	{
		printf("login : OK\n");
		return 1;
	}
	printf("login : %s\n", msg->data);
	return 0;
}

void print(char *p)
{
	int count = 0, len;
	char *q;
	
	printf("   ");
	while ( *p )
	{
		if (*p == ' ' && count == 0) p++;
		printf("%c", *p++);
		count++;

		/* if a new word will begin */
		if (*(p-1) == ' ' && *p != ' ')
		{
			q = p;
			len = 0;
			/* count the length of next word */
			while (*q != ' ' && *q != '\0') 
			{
				len++;
				q++;
			}
            if ((COL - count) < len) 
			{
				printf("\n   ");
				count = 0;
			}
		}
		if (count == 50)
		{
			count = 0;
			printf("\n   ");
		}
	}

	return;
}

void do_query(int socketfd, MSG *msg)
{
	msg->type = Q;
	while ( 1 )
	{
		scanf("%s", msg->data);
		if (strcmp(msg->data, "#") == 0) break;
		send(socketfd, msg, sizeof(MSG), 0);
		recv(socketfd, msg, sizeof(MSG), 0);
		print(msg->data);
		printf("\n");
	}

	return;
}

void do_history(int socketfd, MSG *msg)
{
	msg->type = H;
	send(socketfd, msg, sizeof(MSG), 0);
	while ( 1 )
	{
		recv(socketfd, msg, sizeof(MSG), 0);
		if (msg->data[0] == '\0') break;
		printf("%s\n", msg->data);
	}

	return;
}

int main(int argc, char *argv[])
{
	int socketfd ;
	struct sockaddr_in server_addr;
	MSG msg;
	if (argc < 3)
	{
		printf("Usage : %s <serv_ip> <serv_port>\n", argv[0]);
		exit(-1);
	}
	if ((socketfd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("fail to socket");
		exit(-1);
	}
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(atoi(argv[2]));
	if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("fail to connect");
		exit(-1);
	}
	int n;
	while ( 1 )
	{
		printf("************************************\n");
		printf("* 1: register   2: login   3: quit *\n");
		printf("************************************\n");
		printf("please choose : ");
		if (scanf("%d", &n) <= 0)
		{
			perror("scanf");
			exit(-1);
		}
		switch ( n )
		{
		case 1 :
			printf("\n");
			do_register(socketfd, &msg);
			printf("\n");
			break;
		case 2 :
			printf("\n");
			if (do_login(socketfd, &msg) == 1)
			{
				printf("\n");
				goto next;
			}
			printf("\n");
			break;
		case 3 :
			close(socketfd);
			exit(0);
		}
	}
next:
	while ( 1 )
	{
		printf("***********************************************\n");
		printf("* 1: query_word   2: history_record   3: quit *\n");
		printf("***********************************************\n");
		printf("please choose : ");
		
		if (scanf("%d", &n) < 0)
		{
			perror("scanf");
			exit(-1);
		}
		switch ( n )
		{
		case 1 :
			printf("\n");
			do_query(socketfd, &msg);
			printf("\n");
			break;
		case 2 :
			printf("\n");
			do_history(socketfd, &msg);
			printf("\n");
			break;
		case 3 :
			close(socketfd);
			exit(0);
		}
	}

	return 0;
}