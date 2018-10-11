#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sqlite3.h>
int main(int argc, const char *argv[])
{

	sqlite3 *db=NULL;
	char *errmsg=NULL;

	int ret=sqlite3_open("./dic.db",&db);
	if(ret)
	{
		fprintf(stderr,"fail to sqlite3_open\n");
		return -1;
	}

	char *s="create table dict(word char[128],des char[256])";
	ret=sqlite3_exec(db,s,NULL,NULL,&errmsg);
	if(ret)
	{
		fprintf(stderr,"fail to create table:%s\n",errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	FILE *fp=fopen("./dict.txt","r");
	if(fp==NULL)
	{
		perror("fail to fopen");
		exit(1);
	}
	char buf[512]={0};
	char word[128]={0};
	char des[256]={0};
	char *p=NULL;
	int i=0;
	char sql[512];
			
	while(1)
	{
		bzero(buf,sizeof(buf));
		bzero(word,sizeof(word));
		bzero(des,sizeof(des));
		bzero(sql,sizeof(sql));

		if(NULL==fgets(buf,sizeof(buf),fp))
			break;
		i=0;
		p=buf;
		while(*p!=' ')
		{
			i++;
			p++;
		}

		strncpy(word,buf,i);

		while(*p==' ')
		{
			p++;
		}
		strncpy(des,p,strlen(p)-2);
	//	printf("word:%s des:%s\n",word,des);

		sprintf(sql,"insert into dict values('%s','%s')",word,des);
	//	printf("%s\n",sql);
		ret=sqlite3_exec(db,sql,NULL,NULL,&errmsg);
		if(ret)
		{
			fprintf(stderr,"fail to exec:%s\n",errmsg);
			sqlite3_free(errmsg);
		}



	}

	sqlite3_close(db);
	fclose(fp);

	return 0;
}
