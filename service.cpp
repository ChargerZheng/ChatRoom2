#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include<Windows.h>
#include<hash_map>
#include<iterator>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
#define max_conn_number 20
using namespace std;

typedef struct client {
	int id;
	char *name;
	SOCKET socket;
}Client;
pair<int, Client> Pair;
hash_map<int,Client> clients;
const char *fenge = ":";
const char *wellcome_str = "welcome to connect us,your first input is your name:\0";
int wellcome_length = strlen(wellcome_str);
int conn_number=0;

void *client_run(void *args)
{
	Client *client = (Client *)args;
	if (client->socket == INVALID_SOCKET)
	{
		printf("accept error !");
		closesocket(client->socket);
	}
	send(client->socket, wellcome_str, wellcome_length, 0);
	printf("新线程处理连接\n");
	//将接收第一条数据作为名字
	char name[1024];
	int ret = recv(client->socket, name, 1023, 0);
	if (ret > 0)
	{
		client->name = name;
		name[ret] = 0x00;//添加结束符
	}
	char recvData[1024];
	char *sendData = (char *)malloc(1024 * sizeof(char));
	while (1)
	{
		int ret = recv(client->socket, recvData, 1023, 0);//监听消息
		if (ret < 0)break;//断开连接
		memset(sendData, 0, 1024);
		recvData[ret] = 0x00;//添加结束符
		sprintf(sendData, "%s%s", client->name, fenge);//拼接字符串
		sprintf(sendData, "%s%s", sendData, recvData);//拼接字符串
		for (auto iter = clients.begin(); iter != clients.end(); ++iter) {//群发
			if (iter->first == client->id)continue;
			send(iter->second.socket, sendData, strlen(sendData), 0);
		}
	}
	clients.erase(client->id);
	conn_number--;
	closesocket(client->socket);
	printf("一个连接断开线程执行完毕\n");
	return NULL;
}

void findIP(char *ip, int size)
{
	WORD v = MAKEWORD(1, 1);
	WSADATA wsaData;
	WSAStartup(v, &wsaData); // 加载套接字库

	struct hostent *phostinfo = gethostbyname("");
	char *p = inet_ntoa(*((struct in_addr *)(*phostinfo->h_addr_list)));
	strncpy(ip, p, size - 1);
	ip[size - 1] = '\0';
	WSACleanup();
}


int main(int argc, char* argv[])
{
	//初始化WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)return -1;

	//创建套接字
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//通信协议类型，数据类型，访问方式
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		WSACleanup();
		return -2;
	}

	printf("创建套接字成功！\n");
	char ip[20] = { 0 };
	findIP(ip, sizeof(ip));
	//绑定IP和端口
	sockaddr_in addr;
	addr.sin_family = AF_INET;//协议版本
	addr.sin_port = htons(8888);//
	addr.sin_addr.S_un.S_addr = inet_addr(ip);
	int r = bind(slisten, (LPSOCKADDR)&addr, sizeof(addr));
	if (r == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(slisten);
		return -3;
	}

	printf("绑定套接字成功！IP是:%s。\n",ip);
	if (listen(slisten, 10) == SOCKET_ERROR)
	{
		printf("listen error !");
		return -4;
	}
	//循环接收连接

	int i = 0;//socket数组下标，自增ID
	Client *client;
	pthread_t *thread;
	SOCKADDR remote_addr;
	int remote_addr_rlen=sizeof(remote_addr);
	while (conn_number<=max_conn_number)
	{
		client = (Client *)malloc(sizeof(Client));
		thread= (pthread_t *)malloc(sizeof(pthread_t));
		printf("等待连接...\n");
		client->socket = accept(slisten, &remote_addr, &remote_addr_rlen);//接收连接
		if (client->socket < 0)
		{
			free(client);
			printf("接收失败\n");
			continue;
		}
		client->id = i;
		clients[i] = *client;//把连接存储在hash_map
		printf("接收到一个连接，开启新线程处理\n");
		conn_number++;
		pthread_create(thread, NULL, client_run, &clients[i]);
		i++;
	}

	closesocket(slisten);
	WSACleanup();
	return 0;
}
