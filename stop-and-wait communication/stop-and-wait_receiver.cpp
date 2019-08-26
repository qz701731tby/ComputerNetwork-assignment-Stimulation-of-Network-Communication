#include "Receiver.h"

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
	//cout << r << endl;
	//cout << newString.length() << endl;
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

string handle_msg(char* msg)
{
	string GenXString = "10001000000100001";
	string return_msg = "";
	string processed_msg = "";
	for (int i = 0; i < strlen(msg); i++) {
		processed_msg += msg[i];
	}
	//cout << origin_msg << endl;
	if (isCRC(processed_msg, GenXString) == false) {
		return_msg += '2';
	}
	else if (msg[0] == '0' + expected_frame) {
		if (expected_frame == 0) {
			return_msg += '1';
			expected_frame = 1;
		}
		else {
			return_msg += '0';
			expected_frame = 0;
		}
	}
	return_msg += '\0';
	return  return_msg;
}

int main()
{
	int listen_port;
	listen_port = GetPrivateProfileInt("Network", "ListenPort", 0, ".\\receiver_config.ini");

	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	SOCKET serSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (serSocket == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	//serAddr.sin_port = htons(8888);
	serAddr.sin_port = htons(listen_port);
	serAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(serSocket, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
	{
		printf("bind error !");
		closesocket(serSocket);
		return 0;
	}
	printf("bind succes!\n");

	sockaddr_in remoteAddr;
	int nAddrLen = sizeof(remoteAddr);
	while (true)
	{
		char recvData[100];
		memset(recvData, 0, sizeof(recvData));
		int ret = recvfrom(serSocket, recvData, 100, 0, (sockaddr *)&remoteAddr, &nAddrLen);
		
		expected_char = expected_frame + '0';
		cout << "current expected is:" << expected_char << endl;
		printf(recvData);
		printf("\n");
		
		//�������յ�����Ϣ������ȷ����Ϣ�����ͷ�
		char return_msg[10];
		memset(return_msg, 0, sizeof(return_msg));
		string temp_msg = "";
		temp_msg = handle_msg(recvData);
		for (int i = 0; i < temp_msg.length(); i++) {
			return_msg[i] = temp_msg[i];
		}
		return_msg[1] = '\0';
		//���ԭ����
		if (return_msg[0] != '2') {
			printf("origin_msg is:");
			for (int i = 1; i < 32; i++) {
				printf("%c", recvData[i]);
			}
			printf("\n");
		}

		printf("return_msg:%c\n", return_msg[0]);
		printf("---------------------------------------------------\n");
		sendto(serSocket, return_msg, strlen(return_msg), 0, (sockaddr *)&remoteAddr, nAddrLen);

	}
	closesocket(serSocket);
	WSACleanup();
	return 0;
}
