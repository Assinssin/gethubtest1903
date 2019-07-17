#include "public.h"

void show_userinfo(MSG *msg)
{
	puts("============================");
	printf("%s.\n",msg->recvmsg);
	return ;
}

void admin_query(int sockfd,MSG *msg)
{
	printf("------%s------%d.\n",__func__,__LINE__);
	int n;
	msg->msgtype = ADMIN_QUERY;

	while(1)
	{
		memset(&msg->info,0,sizeof(em_info_t));
		if(msg->usertype == ADMIN){
			printf("******************************************\n");
			printf("****1：按人名查找  2：查找所有 3：退出****\n");
			printf("******************************************\n");
			printf("请输入您的选择（数字）>>");
			scanf("%d",&n);
			getchar();

			switch(n){
				case 1:
					msg->flags = 1;
					break;
				case 2:
					msg->flags = 0;
					break;
				case 3:
					return;
			}
		}
		if(msg->flags == 1){
			printf("请输入您要查找的用户名：");
			scanf("%s",msg->info.username);
			getchar();

			send(sockfd,msg,sizeof(MSG),0);
			recv(sockfd,msg,sizeof(MSG),0);
			printf("工号\t用户类型\t 姓名\t密码\t年龄\n");
			show_userinfo(msg);
		}else{
			send(sockfd,msg,sizeof(MSG),0);
			printf("工号\t用户类型\t 姓名\t密码\t年龄\n");
			while(1){
				recv(sockfd,msg,sizeof(MSG),0);
				if(strncmp(msg->recvmsg,"over*",5) == 0)
					break;
				show_userinfo(msg);
			}
		}
	}
	printf("查找结束\n");
}

void admin_modify(int sockfd,MSG *msg)
{
	printf("------%s------%d.\n",__func__,__LINE__);
	int n = 0;
	msg->msgtype = ADMIN_MODIFY;
	memset(&msg->info,0,sizeof(em_info_t));
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("请输入您要修改职员的工号：");
	scanf("%d",&n);
	getchar();

	printf("*******************请输入要修改的选项**********\n");
	printf("******1：姓名  2：年龄  3：密码  4：退出*******\n");
	printf("***********************************************\n");
	printf("请输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();

	switch(n){
		case 1:
			printf("请输入用户名：");
			msg->recvmsg[0] = 'N';
			scanf("%s",msg->info.username);
			getchar();
			break;
		case 2:
			printf("请输入年龄：");
			msg->recvmsg[0] = 'A';
			scanf("%d",&msg->info.age);
			getchar();
			break;
		case 3:
			printf("请输入新密码：(6位数字)");
			msg->recvmsg[0] = 'D';
			scanf("%6s",msg->info.password);
			getchar();
			break;
		case 4:
			return;
	}
		send(sockfd,msg,sizeof(MSG),0);
		recv(sockfd,msg,sizeof(MSG),0);
		printf("%s",msg->recvmsg);
		printf("修改结束.\n");
}

void admin_adduser(int sockfd,MSG *msg)
{
	printf("------%s------%d.\n",__func__,__LINE__);
	char temp;
	msg->msgtype = ADMIN_ADDUSER;
	msg->usertype = ADMIN;
	memset(&msg->info,0,sizeof(em_info_t));
	while(1){
		printf("欢迎新员工.\n");
		printf("请输入工号：");
		scanf("%d",&msg->info.id);
		getchar();
		printf("您输入的工号是：%d\n",msg->info.id);
		printf("工号信息一旦录入无法更改，请确认您所输入的是否正确！(Y/N)");
		scanf("%c",&temp);
		getchar();
		if(temp == 'N' || temp == 'n'){
			printf("请重新添加用户：");
			break;
		}
		printf("请输入用户名：");
		scanf("%s",msg->info.username);
		getchar();

		printf("请输入用户密码：");
		scanf("%6s",msg->info.password);
		getchar();

		printf("请输入年龄：");
		scanf("%d",&msg->info.age);
		getchar();

		printf("是否为管理员：(Y/N)");
		scanf("%c",&temp);
		getchar();
		if(temp == 'Y' || temp == 'y')
			msg->info.usertype = ADMIN;
		else if(temp == 'N' || temp == 'n')
			msg->info.usertype = USER;
		printf("msg->info.usertype:%d\n",msg->info.usertype);
		
		send(sockfd,msg,sizeof(MSG),0);
		recv(sockfd,msg,sizeof(MSG),0);
		if(strncmp(msg->recvmsg,"OK",2) == 0)
			printf("添加成功！\n");
		else
			printf("%s",msg->recvmsg);
		printf("是否继续添加员工:(Y/N)");
		scanf("%c",&temp);
		getchar();
		if(temp == 'N' || temp == 'n')
			break;
	}
}

void admin_deluser(int sockfd,MSG *msg)
{
	printf("--------------%s--------------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_DELUSER;
	printf("请输入要删除的用户工号：");
	scanf("%d",&msg->info.id);
	getchar();
	printf("请输入要删除的用户名:");
	scanf("%s",msg->info.username);
	getchar();

	send(sockfd, msg, sizeof(MSG), 0);
	recv(sockfd, msg, sizeof(MSG), 0);

	if(strncmp(msg->recvmsg,"OK",2)==0)
		printf("删除成功！\n");
	else
		printf("%s",msg->recvmsg);
	printf("删除工号为：%d 的用户.\n",msg->info.id);
}

void admin_history(int sockfd,MSG *msg)
{
	printf("----------%s-----------%d.\n",__func__,__LINE__);
	msg->msgtype = ADMIN_HISTORY;
	send(sockfd,msg,sizeof(MSG),0);
	while(1){
		recv(sockfd,msg,sizeof(MSG),0);
		if(strncmp(msg->recvmsg,"over*",5) == 0)
			break;
		printf("msg->recvmsg: %s",msg->recvmsg);
	}
	printf("admin查询历史记录结束！\n");
}

void admin_menu(int sockfd,MSG *msg)
{
	int n;
	while(1){
		printf("*************************************************************\n");
		printf("* 1：查询  2：修改 3：添加用户  4:删除用户  5：查询历史记录*\n");
		printf("* 6：退出*\n");
		printf("*************************************************************\n");
		printf("请输入您的选择（数字）>>");
		scanf("%d",&n);
		getchar();
		switch(n)
		{
			case 1:
				admin_query(sockfd,msg);
				break;
			case 2:
				admin_modify(sockfd,msg);
				break;
			case 3:
				admin_adduser(sockfd,msg);
				break;
			case 4:
				admin_deluser(sockfd,msg);
				break;
			case 5:
				admin_history(sockfd,msg);
				break;
			case 6:
				msg->msgtype = QUIT;
				send(sockfd,msg,sizeof(MSG),0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请重新输入！\n");
		}
	}
}

void user_query(int sockfd,MSG *msg)
{
	printf("----------%s----------%d.\n",__func__,__LINE__);
	msg->msgtype = USER_QUERY;
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("工号\t用户类型\t 姓名\t密码\t年龄\n");
	show_userinfo(msg);
}

void user_modify(int sockfd,MSG *msg)
{
	printf("----------%s----------%d.\n",__func__,__LINE__);
	int n = 0;
	msg->msgtype = USER_MODIFY;
	printf("请输入您要修改职员的工号：");
	scanf("%d",&n);
	getchar();

	printf("***********请输入要修改的选项(其他信息亲请联系管理员)*********\n");
	printf("***********       1：年龄   2：密码   3：退出       **********\n");
	printf("**************************************************************\n");
	printf("输入您的选择（数字）>>");
	scanf("%d",&n);
	getchar();

	switch(n){
		case 1:
			printf("请输入年龄：");
			msg->recvmsg[0] = 'D';
			scanf("%d",&msg->info.age);
			getchar();
			break;
		case 2:
			printf("请输入新密码：(6位数字)");
			msg->recvmsg[0] = 'P';
			scanf("%s",msg->info.password);
			getchar();
			break;
		case 3:
			return;
	}
	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("%s",msg->recvmsg);
	printf("修改结束.\n");
}

void user_menu(int sockfd,MSG *msg)
{
	printf("----------%s----------%d.\n",__func__,__LINE__);
	int n;
	while(1){
		printf("*************************************************************\n");
		printf("*****************  1：查询  2：修改  3：退出 ****************\n");
		printf("*************************************************************\n");
		printf("请输入您的选择>>");
		scanf("%d",&n);
		getchar();

		switch(n){
			case 1:
				user_query(sockfd,msg);
				break;
			case 2:
				user_modify(sockfd,msg);
				break;
			case 3:
				msg->msgtype = QUIT;
				send(sockfd,msg,sizeof(MSG),0);
				close(sockfd);
				exit(0);
			default:
				printf("您输入有误，请输入数字\n");
				break;
		}
	}
}

int admin_or_user_login(int sockfd,MSG *msg)
{
	printf("------%s------%d.\n",__func__,__LINE__);
	memset(msg->username,0,NAMELEN);
	printf("请输入用户名：");
	scanf("%s",msg->username);
	getchar();

	memset(msg->password,0,DATALEN);
	printf("请输入密码（6位）");
	scanf("%s",msg->password);
	getchar();

	send(sockfd,msg,sizeof(MSG),0);
	recv(sockfd,msg,sizeof(MSG),0);
	printf("msg->recvmsg :%s",msg->recvmsg);
	
	if(strncmp(msg->recvmsg,"OK",2) == 0){
		if(msg->usertype == ADMIN){
			printf("welcome to 员工管理系统,Admin");
			admin_menu(sockfd,msg);
		}else if(msg->usertype == USER){
			printf("welcome to 员工管理系统,Employee");
			user_menu(sockfd,msg);
		}
	}else{
		printf("登陆失败！%s",msg->recvmsg);
		return -1;
	}
	return 0;
}

int login(int sockfd)
{
	int n;
	MSG msg;
	while(1){
		printf("*************************************************************\n");
		printf("********  1：管理员模式    2：普通用户模式    3：退出********\n");
		printf("*************************************************************\n");
		printf("请输入您的选择>>");
		scanf("%d",&n);
		getchar();

		switch(n){
			case 1:
				msg.msgtype = ADMIN_LOGIN;
				msg.usertype = ADMIN;
				break;
			case 2:
				msg.msgtype = USER_LOGIN;
				msg.usertype = USER;
				break;
			case 3:
				msg.msgtype = QUIT;
				if(send(sockfd,&msg,sizeof(MSG),0) < 0){
					perror("login send");
					return -1;
				}
					close(sockfd);
					exit(0);
				default:
					printf("您的输入有误，请重新输入\n");
		}
		admin_or_user_login(sockfd,&msg);
	}
}


int main(int argc,const char *argv[])
{
	int acceptfd;
	int sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		perror("socket");
		exit(-1);
	}
	printf("sockfd :%d.\n",sockfd);
	ssize_t recvbytes,sendbytes;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	socklen_t addrlen = sizeof(serveraddr);
	socklen_t cli_len = sizeof(clientaddr);
	
	memset(&serveraddr,0,sizeof(serveraddr));
	memset(&clientaddr,0,sizeof(clientaddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(atoi(argv[2]));
	serveraddr.sin_addr.s_addr = inet_addr(argv[1]);

	if(connect (sockfd,(const struct sockaddr *)&serveraddr,addrlen) == -1){
		perror("connect failed.\n");
		exit(-1);
	}
	login(sockfd);
	close(sockfd);
	return 0;

}













