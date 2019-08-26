#ifndef _GBN_CLIENT
#define _GBN_CLIENT

#include<iostream>
#include<process.h>
#include<stdlib.h>
#include<string>
#include<time.h>
#include <winsock2.h>
#include<windows.h>
#include<queue>
#include <sys/timeb.h> 
#pragma comment(lib, "ws2_32.lib")  
#define N 1000
using namespace std;

void recv_thread(void* p);
void timer_thread(void* p);
void start_timer(int frame_num);
void stop_timer(int frame_num);
string int2string(int seq);
long long int f(string s);
string t(long long int a, int length);
long long GetRemainder(string newString, string GenXString);
bool isCRC(string InfoString2, string GenXString);
string GetSendString(string InfoString1, string GenXString);
string prepare_message(string msg);
string make_error(string msg);

struct timer {
	int frame_num;
	string send_msg;
	clock_t start_time;
};

queue<timer> timer_queue;
int msg_count = 0;
int ack;
int send_window_size = 8;
bool timeout = false;
int timelimit = 5;
string msg_queue[N] = { "" };
string origin_msg = "";
string CRC_processed_msg = "";
string prepared_msg = "";
char send_msg[100];
int seq = 0;
int frame_expected = 0;
bool error = false;
int status_flag = 0;
string GenXString = "10001000000100001";

long long int f(string s)//translate a binary string to long
{
	long long int a = 0;
	for (int i = 0; i < s.length(); i++)
	{
		a = a * 2 + (s[i] - '0');
	}
	return a;
}

string t(long long int a, int length)//translate a long to binary 
{
	string s;
	while (length)
	{
		string temp = "1";
		temp[0] = a % 2 + '0';
		s = temp + s;
		a /= 2;
		length--;
	}
	return s;
}

long long GetRemainder(string newString, string GenXString) {
	int r = GenXString.length() - 1;
	string subStr = newString.substr(0, r);
	long long mod = f(subStr);
	long long divisor = f(GenXString);
	for (int i = r; i < newString.length(); i++) {
		mod = (mod << 1) + newString[i] - 48;
		if (mod & 0x10000) {
			mod = mod ^ divisor;
		}
		else {
			mod = mod ^ 0;
		}
	}
	return mod;
}

string GetSendString(string InfoString1, string GenXString)
{
	string newString = InfoString1;
	for (size_t i = 0; i < GenXString.length() - 1; i++)
	{
		newString += "0";
	}
	long long mod = GetRemainder(newString, GenXString);
	string sendString = InfoString1 + t(mod, 16);
	return sendString;
}

bool isCRC(string InfoString2, string GenXString)
{
	long long mod = GetRemainder(InfoString2, GenXString);
	//cout << mod << endl;
	if (mod == 0) {
		return true;
	}
	else {
		return false;
	}
}

void recv_process(char* buf)
{
	string recv_ack = "";
	string recv_seq = "";
	string origin_string = "";
	/*for (int i = 0; i < strlen(buf); i++) {
		origin_string[i] = buf[i];
	}*/
	origin_string = buf;
	cout << "origin_string:" << origin_string<< endl;
	if (isCRC(origin_string, GenXString) == false) {
		error = true;
		printf("this message is error, wait to resend!\n");
	}
	else {
		ack = 0;
		error = false;
		for (int i = 32; i < 35; i++) {
			recv_seq += origin_string[i];			
		}
		cout << "recv_seq:" << recv_seq << endl;
		for (int i = 35; i < 38; i++) {
			recv_ack += origin_string[i];
			ack = ack * 2;
			ack = ack + origin_string[i] - '0';
		}
		cout << "recv_ack:" << recv_ack << endl;
		cout << "ack:" << ack << endl;
		if (int2string(frame_expected) == recv_seq) {
			printf("frame %d receive success!", frame_expected);
			frame_expected = (frame_expected++) % 8;
			//add right_msg to the recv_queue #todo
			if (!timer_queue.empty()) {
				stop_timer((ack + 7) % 8);
			}
		}
		else {
			printf("not expected frame, refuse to receive!\n");
		}	
		
	}

}

//convert int to string for seq and ack
string int2string(int num_seq)
{
	string string_seq;
	switch (num_seq) {
	case 0:
		string_seq = "000"; break;
	case 1:
		string_seq = "001"; break;
	case 2:
		string_seq = "010"; break;
	case 3:
		string_seq = "011"; break;
	case 4:
		string_seq = "100"; break;
	case 5:
		string_seq = "101"; break;
	case 6:
		string_seq = "110"; break;
	case 7:
		string_seq = "111"; break;
	}
	return string_seq;
}

void timer_thread(void* p)
{
	while (true) {
		if (!timeout) {
			if (timer_queue.empty()) {
				Sleep(200);
			}
			else if (!timer_queue.empty()) {
				clock_t nowtime = clock();
				clock_t toptime = timer_queue.front().start_time;
				double dif = ((double)(nowtime - toptime)) / CLK_TCK;
				if (dif >= timelimit) {
					timeout = true;
				}
				Sleep(200);
			}
		}

	}
}

string make_error(string msg)
{
	string new_string = "";
	new_string = msg;
	int length = msg.length();
	int index = rand() % length;
	if (new_string[index] == '0') {
		new_string[index] = '1';
	}
	else if (new_string[index] == '1') {
		new_string[index] = '0';
	}
	return new_string;
}

// process the origin_msg, add seq and ack, CRC, make_error
string prepare_message(string msg)
{
	srand((int)time(0));
	cout << seq << endl;
	string seq_string = int2string(seq);
	string expected_string = int2string(frame_expected);
	cout << "origin string:" << origin_msg << endl;
	cout << "current seq :" << seq_string << endl;
	cout << "expected seq :" << expected_string << endl;

	origin_msg = msg + seq_string + expected_string;
	CRC_processed_msg = GetSendString(origin_msg, GenXString);

	//make_error
	if (rand() % 100 < 20) {
		CRC_processed_msg = make_error(CRC_processed_msg);
		status_flag = 1;
		printf("message %d is supposed to be error!\n", msg_count);
	}

	return CRC_processed_msg;
}

void recv_thread(void * p)
{
	char RecvBuf[1024];//发送数据的缓冲区
	int BufLen = 1024;//缓冲区大小
	SOCKET RecvSocket;
	sockaddr_in RecvAddr;
	sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	//创建接收数据报的socket
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	//将socket与制定端口和0.0.0.0绑定
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(6666);
	RecvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(RecvSocket, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));

	int n = 0;

	while ((n = recvfrom(RecvSocket, RecvBuf, BufLen, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize)) > 0)
	{
		cout << "--------------receiver---------------" << endl;
		RecvBuf[n] = '\0';
		//cout << "receive: " << RecvBuf << endl;
		recv_process(RecvBuf);
		printf("\n");
	}

}

void start_timer(int  frame_num)
{
	struct timer newtimer;
	newtimer.frame_num = frame_num;
	newtimer.start_time = clock();
	newtimer.send_msg = origin_msg;
	//newtimer.send_msg = CRC_processed_msg;
	timer_queue.push(newtimer);
}

void stop_timer(int frame_num) {
	if (timer_queue.empty() != FALSE && timer_queue.front().frame_num == frame_num) {
		timer_queue.pop();
	}
}

#endif