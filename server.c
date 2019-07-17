#include <stdio.h>
#include <sys/types.h>    
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <pthread.h>
#include "public.h" 

sqlite3 *db;
int flags = 0;


void get_system_time(char* timedata)
{
	time_t t;
	struct tm *tp;
	time(&t);
	tp = localtime(&t);
	sprintf(timedata,"%d-%d-%d %d:%d:%d",tp->tm_year+1900,tp->tm_mon+1,\
			tp->tm_mday,tp->tm_hour,tp->tm_min,tp->tm_sec);
	return ;
}

void history_init(MSG *msg,char *buf)
{
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sqlhistory[DATALEN] = {0};
	char timedata[DATALEN] = {0};
	get_system_time(timedata);
	sprintf(sqlhistory,"insert into historyinfo values ('%s','%s','%s');",timedata,msg->username,buf);
	if(sqlite3_exec(db,sqlhistory,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
		printf("insert historyinfo failed.\n");
	}else{
		printf("insert historyinfo success.\n");
	}
}

int admin_or_user_login_request(int acceptfd,MSG *msg)
{
	printf("--------%s--------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char *errmsg;
	char **result;
	int nrow,ncolumn;
	msg->info.usertype = msg->usertype;
	strcpy(msg->info.username,msg->username);
	strcpy(msg->info.password,msg->password);
	printf("usrtype: %#x-----usrname: %s---passwd: %s.\n",msg->info.usertype,msg->info.username,msg->info.password);
	sprintf(sql,"select * from usertypesrinfo where usertype=%d and name='%s' and passwd='%s';",msg->info.usertype,msg->info.username,msg->info.password);
	if(sqlite3_get_table(db,sql,&result,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("---****----%s.\n",errmsg);
	}else{
		if(nrow == 0){
			strcpy(msg->recvmsg,"name or passwd failed.\n");
			send(acceptfd,msg,sizeof(MSG),0);
		}else{
			strcpy(msg->recvmsg,"OK");
			send(acceptfd,msg,sizeof(MSG),0);
		}
	}
	return 0;
}


int user_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int nrow,ncolumn;
	char *errmsg, **resultp;
	char sql[DATALEN] = {0};
	char historybuf[DATALEN] = {0};
	switch (msg->recvmsg[0])
	{
	case 'D':
		sprintf(sql,"update usrinfo set phone='%d' where staffno=%d;",msg->info.age,msg->info.id);
		sprintf(historybuf,"%s修改工号为%d的年龄为%d",msg->username,msg->info.id,msg->info.age);
		break;
	case 'P':
		sprintf(sql,"update usrinfo set passwd='%s' where staffno=%d;",msg->info.password, msg->info.id);
		sprintf(historybuf,"%s修改工号为%d的密码为%s",msg->username,msg->info.id,msg->info.password);
		break;
	}

	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
		sprintf(msg->recvmsg,"数据库修改失败！%s\n", errmsg);
	}else{
		printf("the database is updated successfully.\n");
		sprintf(msg->recvmsg, "数据库修改成功!\n");
		history_init(msg,historybuf);
	}

	send(acceptfd,msg,sizeof(MSG),0);
	printf("--------%s.\n",historybuf);
	return 0;
}

int user_query_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int i = 0,j = 0;
	char sql[DATALEN] = {0};
	char **resultp; 
	int nrow,ncolumn;
	char *errmsg;
	printf(sql,"select * from usrinfo where name='%s';",msg->username);
	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != 	SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching.....\n");
		for(i = 0; i < ncolumn; i++){
			printf("%-8s ",resultp[i]);
		}
		puts("");
		puts("======================================================================================");
		int index = ncolumn;
		for(i = 0; i < nrow; i ++){
			printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
				
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
			puts("=============================================================================");
			index += ncolumn;
		}
		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}						
}

int admin_modify_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int nrow,ncolumn;
	char *errmsg,**resultp;
	char sql[DATALEN] = {0};
	char historybuf[DATALEN] = {0};
	switch(msg->recvmsg[0]){
	case 'N':
		sprintf(sql,"update usrinfo set name='%s' whileere staffno=%d;",msg->info.username, msg->info.id);
		sprintf(historybuf,"sprintf%s修改工号为%d的用户名为%s",msg->username,msg->info.id,msg->info.username);
		break;
	case 'A':
		sprintf(sql,"update usrinfo set age=%d where staffno=%d;",msg->info.age, msg->info.id);
		sprintf(historybuf,"%sä¿®改工号为%d的年龄为%d",msg->username,msg->info.id,msg->info.age);
		break;
	case 'D':
		sprintf(sql,"update usrinfo set passwd='%s' where staffno=%d;",msg->info.password, msg->info.id);
		sprintf(historybuf,"%s修改工号为%dde密码为%s",msg->username,msg->info.id,msg->info.password);
		break;
	}
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
		sprintf(msg->recvmsg,"数据库修改失败！%s", errmsg);
	}else{
		printf("the database is updated successfully.\n");
		sprintf(msg->recvmsg, "数据库修改成功!");
		history_init(msg,historybuf);
	}
	send(acceptfd,msg,sizeof(MSG),0);
	printf("------%s.\n",historybuf);
	return 0;
}

int admin_adduser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char buf[DATALEN] = {0};
	char *errmsg;

	printf("%d\t %d\t %s\t %s\t %d.\n",msg->info.id,msg->info.usertype,msg->info.username,msg->info.password,msg->info.age);
	sprintf(sql,"%d\t %d\t %s\t %s\t %d.\n",msg->info.id,msg->info.usertype,msg->info.username,msg->info.password,msg->info.age);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("-------%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		printf("%s register success.\n",msg->info.username);
	}
	sprintf(buf,"管理员%s添加了%s用户",msg->username,msg->info.username);
	history_init(msg,buf);
	return 0;
}


int admin_deluser_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char buf[DATALEN] = {0};
	char *errmsg;
	printf("msg->info.id :%d\t msg->info.name: %s.\n",msg->info.id,msg->info.username);
	sprintf(sql,"delete from usrinfo where staffno=%d and name='%s';",msg->info.id,msg->info.username);
	if(sqlite3_exec(db,sql,NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("----------%s.\n",errmsg);
		strcpy(msg->recvmsg,"failed");
		send(acceptfd,msg,sizeof(MSG),0);
		return -1;
	}else{
		strcpy(msg->recvmsg,"OK");
		send(acceptfd,msg,sizeof(MSG),0);
		printf("%s deluser %s SUCCESS.\n",msg->info.username,msg->info.username);
	}

	sprintf(buf,"管理员%s删除了%s用户",msg->username,msg->info.username);
	history_init(msg,buf);
	return 0;
}



int admin_query_request(int acceptfd,MSG *msg)
{	
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	int i = 0,j = 0;
	char sql[DATALEN] = {0};
	char **resultp;
	int nrow,ncolumn;
	char *errmsg;

	if(msg->flags == 1){
		sprintf(sql,"select * from usrinfo where name='%s';",msg->info.username);
	}else{
		sprintf(sql,"select * from usrinfo;");
	}

	if(sqlite3_get_table(db, sql, &resultp,&nrow,&ncolumn,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("searching'''''\n");
		printf("ncolumn :%d\tnrow :%d.\n",ncolumn,nrow);
		for(i = 0;i < ncolumn;i++){
			printf("%-8s",resultp[i]);
		}
		puts("");
		puts("=============================================================");

		int index = ncolumn;
		for(i = 0; i < nrow; i++){
			printf("%s    %s     %s     %s     %s     %s     %s     %s     %s     %s     %s.\n",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
				
			sprintf(msg->recvmsg,"%s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s,    %s;",resultp[index+ncolumn-11],resultp[index+ncolumn-10],\
				resultp[index+ncolumn-9],resultp[index+ncolumn-8],resultp[index+ncolumn-7],resultp[index+ncolumn-6],resultp[index+ncolumn-5],\
				resultp[index+ncolumn-4],resultp[index+ncolumn-3],resultp[index+ncolumn-2],resultp[index+ncolumn-1]);
			send(acceptfd,msg,sizeof(MSG),0);
			usleep(1000);
			puts("==============================================");
			index += ncolumn;
		}
		if(msg->flags != 1){
			strcpy(msg->recvmsg,"over*");
			send(acceptfd,msg,sizeof(MSG),0);
		}
		sqlite3_free_table(resultp);
		printf("sqlite3_get_table successfully.\n");
	}
}


int history_callback(void *arg,int ncolumn,char **f_value,char **f_name)
{
	int i = 0;
	MSG *msg = (MSG *)arg;
	int acceptfd = msg->flags;
	if(flags == 0){
		for(i = 0;i < ncolumn;i++){
			printf("%-11s",f_name[i]);
		}
		putchar(10);
		flags = 1;
	}
	for(i = 0;i < ncolumn;i+=3){
		printf("%s-%s-%s",f_value[i],f_value[i+1],f_value[i+2]);
		sprintf(msg->recvmsg,"%s---%s---%s.\n",f_value[i],f_value[i+1],f_value[i+2]);
		send(acceptfd,msg,sizeof(MSG),0);
		usleep(1000);
	}
	puts("");
	return 0;
}

int admin_history_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
	char sql[DATALEN] = {0};
	char *errmsg;
	msg->flags = acceptfd;
	sprintf(sql,"select * form historyinfo;");
	if(sqlite3_exec(db,sql,history_callback,(void *)msg,&errmsg) != SQLITE_OK){
		printf("%s.\n",errmsg); 
	}else{
		printf("query history records done.\n");
	}
	strcpy(msg->recvmsg,"over*");
	send(acceptfd,msg,sizeof(MSG),0);
	flags = 0;
}

int client_quit_request(int acceptfd,MSG *msg)
{
	printf("------------%s-----------%d.\n",__func__,__LINE__);
}

int client_request(int acceptfd,MSG *msg)
{
	printf("------%s------%d.\n",__func__,__LINE__);
	switch (msg->msgtype)
	{
		case USER_LOGIN:
		case ADMIN_LOGIN:
			admin_or_user_login_request(acceptfd,msg);
			break;
		case USER_MODIFY:
			user_modify_request(acceptfd,msg);
			break;
		case USER_QUERY:
			user_query_request(acceptfd,msg);
			break;
		case ADMIN_MODIFY:
			admin_modify_request(acceptfd,msg);
			break;
		case ADMIN_ADDUSER:
			admin_adduser_request(acceptfd,msg);
			break;
		case ADMIN_DELUSER:
			admin_deluser_request(acceptfd,msg);
			break;
		case ADMIN_QUERY:
			admin_query_request(acceptfd,msg);
			break;
		case ADMIN_HISTORY:
			admin_history_request(acceptfd,msg);
			break;
		case QUIT:
			client_quit_request(acceptfd,msg);
			break;
		default:
			break;
	}
}

int main(int argc,const char *argv[])
{
	int acceptfd;
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0){
		perror("socket");
		exit(-1);
	}
	printf("socket................\n");
	ssize_t recvbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);
	MSG msg;
	char *errmsg;
	if(sqlite3_open(STAFF_DATABASE,&db) != SQLITE_OK){
		printf("%s.\n",sqlite3_errmsg(db));
	}else{
		printf("the database open success.\n");
	}

	if(sqlite3_exec(db,"create table usrinfo(staffno integer,usertype integer,name text,passwd text,age integer,phone text,addr text,work text,date text,level integer,salary REAL);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{
		printf("create usrinfo table success.\n");
	}

	if(sqlite3_exec(db,"create table historyinfo(time text,name text,words text);",NULL,NULL,&errmsg)!= SQLITE_OK){
		printf("%s.\n",errmsg);
	}else{ 
		printf("create historyinfo table success.\n");
	}
	
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(bind(sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		printf("bind failed.\n");
		exit(-1);
	}
	printf("bind....................\n");

	if(listen(sockfd,10) == -1){
		perror("listen");
		exit(-1);
	}
	printf("listen..............\n");

	fd_set readfds,tempfds;
	FD_ZERO(&readfds);
	FD_ZERO(&tempfds);
	FD_SET(sockfd,&readfds);
	int mmfd = sockfd;
	int retval;
	int i = 0;

	while(1){
		tempfds = readfds;
		retval = select(mmfd+1,&tempfds,NULL,NULL,NULL);
		if(retval == -1){
			perror("select");
			exit(-1);
		}
		for(i = 0;i < mmfd+1;i++){
			if(FD_ISSET(i,&tempfds)){
				if(i == sockfd){
					acceptfd = accept(sockfd,(struct sockaddr *)&clientaddr,&cli_len);
					if(acceptfd == -1){
						perror("accept");
						exit(-1);
					}
					printf("ip:%s.\n",inet_ntoa(clientaddr.sin_addr));
					FD_SET(acceptfd,&readfds);
					mmfd = mmfd > acceptfd ? mmfd : acceptfd;
				}else{
					recvbytes = recv(i,&msg,sizeof(msg),0);
					printf("msg.type:%#x.\n",msg.msgtype);
					if(recvbytes == -1){
						printf("recv failed.\n");
						continue;
					}else if(recvbytes == 0){
						printf("shutdown.\n");
						close(i);
						FD_CLR(i,&readfds);
					}else{
						client_request(i,&msg);
					}
				}
			}
		}
	}
	close(sockfd);
	return 0;
}	
