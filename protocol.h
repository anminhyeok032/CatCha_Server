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

// 로그인시 패킷
struct CS_LOGIN_PACKET {
	unsigned char	size;
	char			type;
	int 			id;
	char			name[NAME_SIZE];
	char			password[PASSWORD_SIZE];
};
// 이동 및 스킬 사용 키인풋 패킷
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
	int				id;								// 서버에 접속해 있는 캐릭터 번호
	float 			x, y, z;						// 움직인 위치
	float			quat_x, quat_y, quat_z, quat_w;	// 쿼터니언 정보	
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
	float			look_x, look_y, look_z;			// voxel 삭제시 필요한 look 정보
};

///////////////////////////////////////////////
// Server -> Client
///////////////////////////////////////////////

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_LOGIN_INFO_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	result;							// 로그인 결과
};

struct SC_TIME_PACKET {
	unsigned char	size;
	char			type;
	short			time;
};

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_ADD_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int 			id;
	float 			x, y, z;
	float			quat_x, quat_y, quat_z, quat_w;
	uint8_t			character_num;
	char			nickname[NAME_SIZE];
};

// 클라이언트 개인 서버에 접속한 플레이어 정보
struct SC_REMOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;								// 서버에 접속해 있는 캐릭터 번호
	float 			x, y, z;						// 움직인 위치
	float			player_pitch;					// rotate 정보	
	unsigned char	state;							// Object_State와 is_attacked를 합친 값
};

struct SC_SYNC_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	int				id;								// 서버에 접속해 있는 캐릭터 번호
	float 			x, y, z;						// 움직인 위치
	float			quat_x, quat_y, quat_z, quat_w;	// 쿼터니언 정보	
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
	unsigned char	random_seeds[5];				// 5개의 랜덤 시드
};

struct SC_REMOVE_VOXEL_SPHERE_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	cheese_num;						// 치즈 번호, 치즈 전부 삭제 여부
	float			center_x, center_y, center_z;	// 삭제할 복셀 중심
};

struct SC_SET_MY_ID_PACKET {
	unsigned char	size;
	char			type;
	unsigned char	my_id;							// 자신의 아이디 번호
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