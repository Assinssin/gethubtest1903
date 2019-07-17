#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sqlite3.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sqlite3.h>

#define STAFF_DATABASE "staff_manage_system.db"
#define QUIT          0x11111111

#define USER_LOGIN    0x00000000
#define USER_MODIFY   0x00000001
#define USER_QUERY    0x00000002

#define ADMIN_LOGIN   0x10000000
#define ADMIN_MODIFY  0x10000001
#define ADMIN_ADDUSER 0x10000002
#define ADMIN_DELUSER 0x10000004
#define ADMIN_QUERY   0x10000008
#define ADMIN_HISTORY 0x10000010

#define ADMIN 0
#define USER 1

#define NAMELEN 16
#define DATALEN 128

typedef struct em_info{
	int  id;            //employee number
	int  usertype;   	//ADMIN or USER
	char username[NAMELEN];
	char password[6];
	int  age;
}em_info_t;

//employees message
typedef struct {
	int msgtype;
	int usertype;
	char username[NAMELEN];
	char password[6];
	char recvmsg[DATALEN];
	int flags;
	void *released;
	em_info_t info;
}MSG;

typedef struct read_data{
	int acceptfd;
	pthread_t thread;
	int state;
	MSG *msg;
	void *prvi_data;
}read_data_t;

typedef struct read_node{
	read_data_t data;
	struct read_node *next;
}linklist,*plinklist;

#endif
