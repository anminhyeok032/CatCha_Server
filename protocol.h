#pragma once

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 25;
constexpr int PASSWORD_SIZE = 25;

constexpr int W_WIDTH = 8;
constexpr int W_HEIGHT = 8;

#define MAXUSER = 4;

// Packet ID
// Client -> Server
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

// Server -> Client
constexpr char SC_LOGIN_INFO = 11;
constexpr char SC_ADD_PLAYER = 12;
constexpr char SC_REMOVE_PLAYER = 13;
constexpr char SC_MOVE_PLAYER = 14;
constexpr char SC_REBIRTH_PACKET = 15;

#pragma pack (push, 1)
///////////////////////////////////////////////
// Client -> Server
///////////////////////////////////////////////

// �α��ν� ��Ŷ
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	char	name[NAME_SIZE];
	char	password[PASSWORD_SIZE];
};
// �̵� �� ��ų ��� Ű��ǲ ��Ŷ
struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	uint8_t	keyinput;
	short	camera_yaw, player_yaw;	
};


///////////////////////////////////////////////
// Server -> Client
///////////////////////////////////////////////

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	float 	x, y, z;	// ���� ��Ī �ý��� ���� �� ������ �߰� �ؾ���
};

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	float 	x, y, z;
	char	nickname[NAME_SIZE];
};

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;				// ������ ������ �ִ� ĳ���� ��ȣ
	float 	x, y, z;		// ������ ��ġ
	float	yaw;	// look vector
};

// ���� ���� �� ��ų Ű ��ǲ, ��� ��ų
struct SC_REBIRTH_PACKET {
	unsigned char size;
	char	type;
	int		id;
	int		ai_num;
};
#pragma pack (pop)