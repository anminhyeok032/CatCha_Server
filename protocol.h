#pragma once
constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 25;
constexpr int PASSWORD_SIZE = 25;

// Packet ID
// Client -> Server
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_TIME = 2;
constexpr char CS_ROTATE = 3;
constexpr char CS_SYNC_PLAYER = 4;
constexpr char CS_CHOOSE_CHARACTER = 5;
constexpr char CS_VOXEL_LOOK = 6;

// Server -> Client
constexpr char SC_LOGIN_INFO = 11;
constexpr char SC_ADD_PLAYER = 12;
constexpr char SC_REMOVE_PLAYER = 13;
constexpr char SC_MOVE_PLAYER = 14;
constexpr char SC_CHANGE_CHARACTER = 15;
constexpr char SC_TIME = 16;
constexpr char SC_SYNC_PLAYER = 17;
constexpr char SC_RANDOM_VOXEL_SEED = 18;
constexpr char SC_REMOVE_VOXEL_SPHERE = 19;
constexpr char SC_SET_MY_ID = 20;
constexpr char SC_GAME_START = 21;
constexpr char SC_GAME_OPEN_DOOR = 22;
constexpr char SC_GAME_WIN_CAT = 23;
constexpr char SC_GAME_WIN_MOUSE = 24;
constexpr char SC_AI_MOVE = 25;

#pragma pack (push, 1)
///////////////////////////////////////////////
// Client -> Server
///////////////////////////////////////////////

// �α��ν� ��Ŷ
struct CS_LOGIN_PACKET {
	unsigned char	size;
	char			type;
	int 			id;
	char			name[NAME_SIZE];
	char			password[PASSWORD_SIZE];
};
// �̵� �� ��ų ��� Ű��ǲ ��Ŷ
struct CS_MOVE_PACKET {
	unsigned char	size;
	char			type;
	uint8_t			keyinput;
};

struct CS_ROTATE_PACKET {
	unsigned char	size;
	char			type;
	float			player_pitch;
};

struct CS_SYNC_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;								// ������ ������ �ִ� ĳ���� ��ȣ
	float 			x, y, z;						// ������ ��ġ
	float			quat_x, quat_y, quat_z, quat_w;	// ���ʹϾ� ����	
};

struct CS_CHOOSE_CHARACTER_PACKET {
	unsigned char	size;
	char			type;
	bool			is_cat;							// true : Cat, false : Mouse
};

struct CS_TIME_PACKET {
	unsigned char	size;
	char			type;
	unsigned short	time;
};

struct CS_VOXEL_LOOK_PACKET {
	unsigned char	size;
	char			type;
	float			look_x, look_y, look_z;			// voxel ������ �ʿ��� look ����
};

///////////////////////////////////////////////
// Server -> Client
///////////////////////////////////////////////

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_LOGIN_INFO_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	result;							// �α��� ���
};

struct SC_TIME_PACKET {
	unsigned char	size;
	char			type;
	short			time;
};

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_ADD_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int 			id;
	float 			x, y, z;
	float			quat_x, quat_y, quat_z, quat_w;
	uint8_t			character_num;
	char			nickname[NAME_SIZE];
};

// Ŭ���̾�Ʈ ���� ������ ������ �÷��̾� ����
struct SC_REMOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;								// ������ ������ �ִ� ĳ���� ��ȣ
	float 			x, y, z;						// ������ ��ġ
	float			player_pitch;					// rotate ����	
	unsigned char	state;							// Object_State�� is_attacked�� ��ģ ��
};

struct SC_SYNC_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;								// ������ ������ �ִ� ĳ���� ��ȣ
	float 			x, y, z;						// ������ ��ġ
	float			quat_x, quat_y, quat_z, quat_w;	// ���ʹϾ� ����	
};

struct SC_CHANGE_CHARACTER_PACKET {
	unsigned char	size;
	char			type;
	uint8_t			player_num;
	uint8_t			prev_character_num;
	uint8_t			new_character_num;
};

struct SC_RANDOM_VOXEL_SEED_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	random_seeds[5];				// 5���� ���� �õ�
};

struct SC_REMOVE_VOXEL_SPHERE_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	cheese_num;						// ġ�� ��ȣ, ġ�� ���� ���� ����
	float			center_x, center_y, center_z;	// ������ ���� �߽�
};

struct SC_SET_MY_ID_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	my_id;							// �ڽ��� ���̵� ��ȣ
};

struct SC_GAME_STATE_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	winner;
};

struct SC_AI_MOVE_PACKET {
	unsigned char	size;
	char			type;
	int				id;
	float			x, z;
};
#pragma pack (pop)