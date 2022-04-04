#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>

#include "Protocol.h"
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;

constexpr int MAX_USER = 10;

enum COMP_TYPE
{
	OP_ACCEPT,
	OP_RECV,
	OP_SEND,
};

class OVER_EXP {
public:
	// _over의 주소 == SEND_DATA 인스턴스의 주소
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;

	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}

	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		_comp_type = OP_SEND;
		ZeroMemory(&_over, sizeof(_over));
		memcpy(_send_buf, packet, packet[0]);
	}
};

class SESSION {
	OVER_EXP _recv_over;

public:
	bool in_use;
	int _id;
	SOCKET _socket;
	short x, y;
	char _name[NAME_SIZE];
	int _prev_remain;

public:
	SESSION()
	{
		in_use = false;
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag, &_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &(sdata->_wsabuf), 1, 0, 0, &(sdata->_over), 0);
	}

	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.id = _id;
		p.x = x;
		p.y = y;
		do_send(&p);
	}

	void send_move_packet()
	{
		SC_MOVE_PLAYER_PACKET p;
		p.size = sizeof(SC_MOVE_PLAYER_PACKET);
		p.type = SC_MOVE_PLAYER;
		p.id = _id;
		p.x = x;
		p.y = y;
		do_send(&p);
	}
};

array<SESSION, MAX_USER> clients;

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (clients[i].in_use == false)
		{
			return i;
		}
	}

	return -1;
}

void processPacket(int c_id, char* packet)
{
	switch (packet[1])
	{
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		clients[c_id].send_login_info_packet();
		break;
	}
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short& x = clients[c_id].x;
		short& y = clients[c_id].y;
		switch (p->direction)
		{
		case 0: // UP
			if (y > 0) 
				y--;
			break;
		case 1: // DOWN
			if(y < W_HEIGHT - 1)
				y++;
			break;
		case 2: // LEFT
			if(x > 0)
				x--;
			break;
		case 3: // RIGHT
			if(x < W_WIDTH - 1)
				x++;
			break;
		}
		clients[c_id].send_move_packet();
		break;
	}
	default:
		break;
	}
}

int main()
{
	HANDLE h_iocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);

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

	::CreateIoCompletionPort(reinterpret_cast<HANDLE>(server), h_iocp, 9999, 0);

	SOCKET c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	OVER_EXP a_over;
	a_over._comp_type = OP_ACCEPT;

	AcceptEx(server, c_socket, a_over._send_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
		0, &a_over._over);

	while (true) {
		DWORD num_bytes = 0;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;

		::GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);

		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);

		switch (ex_over->_comp_type)
		{
		case OP_ACCEPT:
		{
			int client_id = get_new_client_id();
			if (client_id != -1)
			{
				::CreateIoCompletionPort(reinterpret_cast<HANDLE>(c_socket), h_iocp, client_id, 0);
				clients[client_id].in_use = true;
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = c_socket;
				clients[client_id].do_recv();
				c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else
			{
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&a_over._over, sizeof(a_over._over));
			AcceptEx(server, c_socket, a_over._send_buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
				0, &a_over._over);
			break;
		}
		case OP_RECV:
		{
			int remain_data = num_bytes + clients[key]._prev_remain;

			char* p = ex_over->_send_buf;
			while (remain_data > 0)
			{
				int packet_size = p[0];
				if (packet_size <= remain_data)
				{
					processPacket(static_cast<int>(key), p);
					p += packet_size;
					remain_data -= packet_size;
				}
				else
				{
					break;
				}
			}

			clients[key]._prev_remain = remain_data;

			if (remain_data > 0)
			{
				memcpy(ex_over->_send_buf, p, sizeof(remain_data));
			}

			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;

		default:
			break;
		}
	}
	closesocket(server);
	WSACleanup();
}
