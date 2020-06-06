#define _SILENCE_STDEXT_HASH_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<stdio.h>
#include<stdlib.h>
#include <pthread.h>
#include<Windows.h>
#include<string.h>
#include<map>
#include<hash_map>
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "pthreadVC2.lib")
void *receive_message(void *args)
{
	SOCKET *sClient = (SOCKET *)args;
	char *recvData = (char *)malloc(1024 * sizeof(char));
	recvData[1023]='\0';
	int ret;
	while(1)
	{
		ret = recv(*sClient, recvData, 1023, 0);
		if (ret < 0)break;
		recvData[ret] = 0x00;//添加结束符
		printf("%s\n", recvData);
	}
	free(recvData);
	closesocket(*sClient);
	return NULL;
}

int main(int argc, char* argv[])
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)return -1;
	printf("初始化WSA成功！\n");

	SOCKET sClient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//通信协议类型，数据类型，访问方式
	if (sClient == INVALID_SOCKET)
	{
		printf("socket error !");
		WSACleanup();
		return -2;
	}

	char server_ip[20] = { 0 };
	printf("创建套接字成功！请输入服务器IP：\n");
	scanf("%s", &server_ip);
	//获取服务器协议族
	sockaddr_in addr;
	addr.sin_family = AF_INET;//协议版本
	addr.sin_port = htons(8888);//
	addr.sin_addr.S_un.S_addr = inet_addr(server_ip);
	if (connect(sClient, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
	{
		printf("connect error !");
		closesocket(sClient);
		return -3;
	}

	printf("连接服务器（%s）成功！\n",server_ip);
	pthread_t thread;
	pthread_create(&thread, NULL, receive_message, (void *)&sClient);//开启新线程，接收服务器发送的消息

	char *sendData = (char *)malloc(1024 * sizeof(char));;
	sendData[1023] = '\0';
	while (true)
	{
		memset(sendData, 0, 1024);
		scanf("%s", sendData);
		if (sendData == "quit\0")break;
		send(sClient, sendData, strlen(sendData), NULL);
	}
	free(sendData);
	closesocket(sClient);
	WSACleanup();
	return 0;
}