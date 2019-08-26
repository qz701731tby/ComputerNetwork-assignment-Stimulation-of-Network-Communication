#ifndef __SENDER_H
#define __SENDER_H

#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<time.h>
#include <winsock2.h>
#include<Windows.h>
#include<iostream>
#pragma comment(lib, "ws2_32.lib")  
using namespace std;
#define N 1000
string msg_queue[N] = { "" };
string origin_msg = "";
string CRC_processed_msg = "";
char prepared_msg[100] ;
char recv_msg[100];
char char_seq;
struct sockaddr_in server;
int serverfd;
int next_frame_to_send = 1;//下一个发送的帧编号
int status_flag = 0;//发送状态标志位
int msg_count = 0;//标记送到第几帧
int seq = 0;//当前帧的编号
int recv_flag = 0;//0表示未接受，1表示成功接收返回信息

#endif
