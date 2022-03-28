#include <iostream>
#include <unordered_map>
#include <WS2tcpip.h>

#include "Session.h"

#pragma comment(lib, "WS2_32.lib")
using namespace std;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

unordered_map<int, Session> gClients;
unordered_map<WSAOVERLAPPED*, int> gOverToID;


void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);

void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int id = gOverToID[over];
	
	Session& client = gClients[id];
	client.do_recv();
}

void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags)
{
	int id = gOverToID[over];

	if (0 == num_bytes)
	{
		cout << "Client disconnected" << endl;

		gOverToID.erase(over);
		gClients.erase(id);
	}

	Session& client = gClients[id];

	cout << "Client sent: " << client.mMess << endl;

	client.do_send(num_bytes);
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(server, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(server, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int clientID = 0;

	while (true)
	{
		SOCKET client = WSAAccept(server, reinterpret_cast<sockaddr*>(&cl_addr), &addr_size, 0, 0);
		gClients.try_emplace(clientID, clientID, client);
		gClients[clientID].do_recv();

		++clientID;
	}

	closesocket(server);
	WSACleanup();
}
