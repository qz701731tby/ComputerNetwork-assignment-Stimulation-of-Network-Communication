#include "Sender.h"

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

void handle_returnMsg(char msg)
{
	if ((msg - '0') == next_frame_to_send) {
		printf("receive success!\n");
		if (next_frame_to_send == 0) {
			seq = 0;
			next_frame_to_send = 1;
		}
		else if (next_frame_to_send == 1) {
			seq = 1;
			next_frame_to_send = 0;
		}
		msg_count++;
		recv_flag = 0;
	}
	else if (msg == NULL) {
		printf("Package lost, resend temp package!\n");
	}
	else if (msg == '2') {
		printf("error occur during the send!\n");
	}
}

string make_error(string msg)
{
	string new_string = "";
	new_string = msg;
	int length = 48;
	int index = rand() % 48;
	if (new_string[index] == '0') {
		new_string[index] = '1';
	}
	else if (new_string[index] == '1') {
		new_string[index] = '0';
	}
	return new_string;
}

int main()
{
	char serverIP[20];
	int timeout;
	int listen_port;
	//read parameters from config.ini
	GetPrivateProfileString("Network", "ServerIP", "", serverIP, 20, ".\\sender_config.ini");
	timeout = GetPrivateProfileInt("Network", "timeout", 0, ".\\sender_config.ini");
	listen_port = GetPrivateProfileInt("Network", "ListenPort", 0, ".\\sender_config.ini");

	srand((int)time(0));
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}
	SOCKET sclient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	//sin.sin_port = htons(8888);
	sin.sin_port = htons(listen_port);
	//sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	sin.sin_addr.S_un.S_addr = inet_addr(serverIP);
	int len = sizeof(sin);

	//set the receive timeout
	struct timeval tv;
	int ret;
	tv.tv_sec = timeout * 1000;
	//tv.tv_sec = 5000;
	tv.tv_usec = 0;
	if (setsockopt(sclient, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv)) < 0) {
		printf("socket option  SO_RCVTIMEO not support\n");
		return 0;
	}

	string GenXString = "10001000000100001";
	//generate the msg queue
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 31; j++) {
			msg_queue[i] += rand() % 2 + '0';
		}
	}

	while (true) {
		memset(prepared_msg, 0, sizeof(prepared_msg));
		if (msg_queue[msg_count] == "") {
			break;
		}
		else {
			origin_msg = msg_queue[msg_count];
		}
		cout <<"origin_msg is: "<< origin_msg << endl;
		char_seq = seq + '0';
		cout << "current seq is: " << char_seq<<endl;
		origin_msg = char_seq + origin_msg;
		CRC_processed_msg = GetSendString(origin_msg, GenXString);

		cout <<"send_msg is: "<< CRC_processed_msg << endl;
		if (rand() % 100 < 20) {
			CRC_processed_msg = make_error(CRC_processed_msg);
			status_flag = 1;
			printf("message %d is supposed to be error!\n", msg_count);
		}

		//convert string to char*
		for (int i = 0; i < CRC_processed_msg.length(); i++) {
			prepared_msg[i] = CRC_processed_msg[i];
		}
		//cout << CRC_processed_msg << endl;
		//not to send msg in a certain propability
		if (rand() % 100 < 20 && status_flag != 1) {
			status_flag = 2;
			printf("msg %d is suppoesd to be lost.\n", msg_count);
		}
		else {
			if (sendto(sclient, prepared_msg, strlen(prepared_msg), 0, (sockaddr *)&sin, len)) {
				printf("message %d: send successfully!\n", msg_count);
			}
		}

		char recvData[10];
		memset(recvData, 0, sizeof(recvData));
		int ret = recvfrom(sclient, recvData, 10, 0, (sockaddr *)&sin, &len);
		printf("return_msg from receiver: %c\n", recvData[0]);
		handle_returnMsg(recvData[0]);
		printf("--------------------------------------------------------------\n");
		Sleep(2000);
	}

	closesocket(sclient);
	WSACleanup();
	return 0;

}


