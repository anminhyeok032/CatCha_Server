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
constexpr char CS_TIME = 2;
constexpr char CS_ROTATE = 3;
constexpr char CS_SYNC_PLAYER = 4;
constexpr char CS_CHOOSE_CHARACTER = 5;

// Server -> Client
constexpr char SC_LOGIN_INFO = 11;
constexpr char SC_ADD_PLAYER = 12;
constexpr char SC_REMOVE_PLAYER = 13;
constexpr char SC_MOVE_PLAYER = 14;
constexpr char SC_CHANGE_CHARACTER = 15;
constexpr char SC_TIME = 16;
constexpr char SC_SYNC_PLAYER = 17;

#pragma pack (push, 1)
///////////////////////////////////////////////
// Client -> Server
///////////////////////////////////////////////

// 로그인시 패킷
struct CS_LOGIN_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	char	name[NAME_SIZE];
	char	password[PASSWORD_SIZE];
};
// 이동 및 스킬 사용 키인풋 패킷
struct CS_MOVE_PACKET {
	unsigned char size;
	char	type;
	uint8_t	keyinput;
};

struct CS_ROTATE_PACKET {
	unsigned char size;
	char type;
	float player_pitch;
};

struct CS_SYNC_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;				// 서버에 접속해 있는 캐릭터 번호
	float 	x, y, z;		// 움직인 위치
	float	look_x, look_y, look_z;	// rotate 정보	
};

struct CS_CHOOSE_CHARACTER_PACKET {
	unsigned char size;
	char	type;
	bool	is_cat;			// true : Cat, false : Mouse
};

struct CS_TIME_PACKET {
	unsigned char size;
	char type;
	unsigned short time;
};

///////////////////////////////////////////////
// Server -> Client
///////////////////////////////////////////////

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_LOGIN_INFO_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	float 	x, y, z;	
};

struct SC_TIME_PACKET {
	unsigned char size;
	char type;
	unsigned short time;
};

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_ADD_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int 	id;
	float 	x, y, z;
	char	nickname[NAME_SIZE];
};

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_REMOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;				// 서버에 접속해 있는 캐릭터 번호
	float 	x, y, z;		// 움직인 위치
	float	player_pitch;	// rotate 정보	
};

struct SC_SYNC_PLAYER_PACKET {
	unsigned char size;
	char	type;
	int		id;				// 서버에 접속해 있는 캐릭터 번호
	float 	x, y, z;		// 움직인 위치
	float	look_x, look_y, look_z;	// rotate 정보	
};

struct SC_CHANGE_CHARACTER_PACKET {
	unsigned char size;
	char	type;
	int		id;
	uint8_t prev_character_num;
	uint8_t new_character_num;
};

#pragma pack (pop)