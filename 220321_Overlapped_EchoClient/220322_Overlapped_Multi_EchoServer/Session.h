#pragma once

#include <WS2tcpip.h>
#include <unordered_map>

using std::unordered_map;

extern void CALLBACK recv_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
extern void CALLBACK send_callback(DWORD err, DWORD num_bytes, LPWSAOVERLAPPED over, DWORD flags);
extern unordered_map<WSAOVERLAPPED*, int> gOverToID;

class Session
{
public:
	Session() = default;

	Session(int id, SOCKET s)
		: mID(id),
		mSocket(s)
	{
		mWsabuf[0].buf = mMess;
		mWsabuf[0].len = sizeof(mMess);
		gOverToID[&mOver] = mID;
	}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&mOver, 0, sizeof(mOver));
		WSARecv(mSocket, mWsabuf, 1, 0, &recv_flag, &mOver, recv_callback);
	}

	void do_send(int numBytes)
	{
		mWsabuf[0].len = numBytes;
		memset(&mOver, 0, sizeof(mOver));
		WSASend(mSocket, mWsabuf, 1, 0, 0, &mOver, send_callback);
	}
	
public:
	int mID;
	SOCKET mSocket;
	WSAOVERLAPPED mOver;
	WSABUF mWsabuf[1];
	CHAR mMess[200];
};

