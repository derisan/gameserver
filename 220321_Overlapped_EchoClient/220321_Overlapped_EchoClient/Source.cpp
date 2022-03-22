#include <iostream>
#include <WS2tcpip.h>
using namespace std;
#pragma comment (lib, "WS2_32.LIB")
const char* SERVER_ADDR = "127.0.0.1";
const short SERVER_PORT = 4000;
const int BUFSIZE = 256;

char recv_buf[BUFSIZE];
char send_buf[BUFSIZE];
SOCKET s_socket;
WSABUF r_wsabuf;
WSABUF s_wsabuf;
DWORD recv_flag = 0;

void do_recv(SOCKET s_socket);

void error_display(const char* msg, int err_no)
{
	WCHAR* lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"¿¡·¯ " << lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
}

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	delete over;
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	cout << "Server Sent [" << num_bytes << "bytes] : " << recv_buf << endl;
	delete over;

	do_recv(s_socket);
}

void do_recv(SOCKET s_socket)
{
	r_wsabuf.buf = recv_buf;
	r_wsabuf.len = BUFSIZE;

	recv_flag = 0;

	WSAOVERLAPPED* r_over = new WSAOVERLAPPED;
	ZeroMemory(r_over, sizeof(WSAOVERLAPPED));

	int retVal = WSARecv(s_socket, &r_wsabuf, 1, 0, &recv_flag, r_over, recv_callback);

	if (retVal == SOCKET_ERROR)
	{
		int err_no = WSAGetLastError();

		if (err_no != WSA_IO_PENDING)
		{
			error_display("do_recv(): ", WSAGetLastError());
		}
	}
}

int main()
{
	wcout.imbue(locale("korean"));
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	s_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	ZeroMemory(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr);
	connect(s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

	do_recv(s_socket);

	for (;;) {
		cout << "Enter Message : "; 
		cin.getline(send_buf, BUFSIZE);

		s_wsabuf.buf = send_buf;
		s_wsabuf.len = static_cast<ULONG>(strlen(send_buf)) + 1;

		WSAOVERLAPPED* s_over = new WSAOVERLAPPED;
		ZeroMemory(s_over, sizeof(WSAOVERLAPPED));

		WSASend(s_socket, &s_wsabuf, 1, 0, 0, s_over, send_callback);

		SleepEx(100, true);
	}

	closesocket(s_socket);
	WSACleanup();
}
