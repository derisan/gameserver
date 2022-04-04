#pragma once

constexpr int NAME_SIZE = 20;
constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

/**
 * Packet ID
 */
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;


/**
 * Packet struct
 */

#pragma pack(push, 1)
struct CS_LOGIN_PACKET
{
	unsigned char size;
	char type;
	char name[NAME_SIZE];
};

struct CS_MOVE_PACKET
{
	unsigned char size;
	char type;
	char direction;
};

struct SC_LOGIN_INFO_PACKET
{
	unsigned char size;
	char type;
	short id;
	short x, y;
};

struct SC_ADD_PLAYER_PACKET
{
	unsigned char size;
	char type;
	short id;
	short x, y;
	char name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	short id;
};

struct SC_MOVE_PLAYER_PACKET
{
	unsigned char size;
	char type;
	short id;
	short x, y;
};

#pragma pack(pop)