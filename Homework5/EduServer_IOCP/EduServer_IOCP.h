#pragma once


#define LISTEN_PORT		9990
#define MAX_CONNECTION	10000

#define CONNECT_SERVER_ADDR	"127.0.0.1"
#define CONNECT_SERVER_PORT 9001

//DONE: SQL ���� ��Ʈ�� �������� ��Ȳ�� �°� ����
#define SQL_SERVER_CONN_STR	L"Driver={SQL Server};Server=NAMHYEONUK-PC;Database=GameDB;UID=sa;PWD=nextuser" 
///# Server=�������ּ�\\SQL�ν��Ͻ��̸� <<-- �̰� ����..

#define GQCS_TIMEOUT	10 //INFINITE

enum THREAD_TYPE
{
	THREAD_MAIN,
	THREAD_IO_WORKER
};

extern __declspec(thread) int LThreadType;