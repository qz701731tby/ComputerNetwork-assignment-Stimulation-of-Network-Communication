#include "GBNclient.h"

int main()
{
	srand((int)time(0));
	WORD socketVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(socketVersion, &wsaData) != 0)
	{
		return 0;
	}
	SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8888);
	//sin.sin_port = htons(listen_port);
	sin.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	//sin.sin_addr.S_un.S_addr = inet_addr(serverIP);
	int len = sizeof(sin);

	//generate the msg queue
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < 32; j++) {
			msg_queue[i] += rand() % 2 + '0';
		}
	}

	_beginthread(recv_thread, 1024 * 1024, NULL);
	_beginthread(timer_thread, 1024 * 1024, NULL);

	Sleep(5000);

	while (true)
	{
		cout << "-----------sender------------" << endl;

		if (timeout) {
			int timeout_ack = timer_queue.front().frame_num;
			seq = timeout_ack;
			printf("receive ack[%d] timeout,resend msg!", timeout_ack);
			while (!timer_queue.empty()) {
				string resend_msg = timer_queue.front().send_msg;
				timer_queue.pop();
				prepared_msg = prepare_message(resend_msg);

				for (int i = 0; i < CRC_processed_msg.length(); i++) {
					send_msg[i] = prepared_msg[i];
				}

				sendto(sock, send_msg, strlen(send_msg), 0, (sockaddr *)&sin, len);
				printf("message %d resend successfully!\n", seq);
				seq = (seq++)%8;
				Sleep(1000);
			}
			timeout = false;
		}
		else {
			memset(send_msg, 0, sizeof(send_msg));
			if (msg_queue[msg_count] == "") {
				break;
			}
			else {
				origin_msg = msg_queue[msg_count];
			}

			prepared_msg = prepare_message(origin_msg);

			//convert string to char* which can be sent by socket
			for (int i = 0; i < prepared_msg.length(); i++) {
				send_msg[i] = prepared_msg[i];
			}
			
			
			if (rand() % 100 < 20 && status_flag != 1) {
				status_flag = 2;
				start_timer(seq);
				printf("msg %d is suppoesd to be lost.\n", msg_count);
			}
			else {
				if (sendto(sock, send_msg, strlen(send_msg), 0, (sockaddr *)&sin, len)) {
					start_timer(seq);
					printf("message %d send successfully!\n", msg_count);		
					cout << send_msg << endl;
				}
			}

			msg_count++;
			seq = (seq++) % 8;
			if (msg_count == 20) {
				break;
			}
			
		}
		
		status_flag = 0;
		printf("\n");
		Sleep(1000);
	}
	closesocket(sock);
	WSACleanup();
	return 0;
}


