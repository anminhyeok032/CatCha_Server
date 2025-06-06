#pragma once

#include <iostream>
#include <array>
#include <thread>
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <tchar.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include "protocol.h"

#include <chrono>
#include <random>

#include <concurrent_queue.h>
#include <concurrent_priority_queue.h>
#include <concurrent_unordered_map.h>

#include <cmath>

#include <bitset>

#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

constexpr short PORT = 4000;
constexpr int BUFSIZE = 256;
constexpr int SESSION_MAX_USER = 5;
constexpr int SESSION_MAX_NPC = 4;

constexpr int MAX_USER = SESSION_MAX_USER * 1000;
constexpr int MAX_NPC = 4000;
constexpr short GAME_TIME = 300;
constexpr short FREEZING_TIME = 3;				// 멈춤 고정 시간

// 고정 시간 스텝 설정 (1/60초)
constexpr float FIXED_TIME_STEP = 1.0f / 60.0f;
constexpr float UPDATE_PERIOD = 1.0f / 60.0f;
constexpr int	UPDATE_PERIOD_INT = static_cast<int>(UPDATE_PERIOD * 1000);
constexpr int	AI_UPDATE_PERIOD_INT = 1000;

// cat
constexpr float JUMP_END_TIME = 0.3f;
constexpr float CAT_ATTACK_TIME = 1.33333337f / 2.0f;
constexpr float CAT_PUNCH_POWER = 250.0f;
constexpr int	CAT_ATTACK_DAMAGE = 50;
constexpr float CAT_STUN_TIME = 1.6666666f / 2.0f;
constexpr float CHARGING_JUMP_FORCE = 300.0f;
constexpr float CAT_MAX_JUMP_CHARGING_TIME = 2.0f;
// mouse
constexpr float MOUSE_BITE_TIME = 0.416666657f;
constexpr float MOUSE_BITE_SIZE = 4.0f;
constexpr float MOUSE_REBORN_TIME = 5.0f;
constexpr float MOUSE_WALL_WALKING_VELOCITY = 50.0f;

// AI를 위한 맵 설정
constexpr float FLOOR_Y = -61.6f;
constexpr int TILE_SIZE = 10;
constexpr int TILE_MAP_WIDTH = 1200;
constexpr int TILE_MAP_LENGTH = 1200;
constexpr int TILE_X_CORR = TILE_MAP_LENGTH / TILE_SIZE;


struct CompletionKey
{
	int* session_id;
	int* player_index;
};

enum IO_TYPE
{
	IO_ACCEPT = 0,
	IO_SEND,
	IO_RECV,
	IO_MOVE,
	IO_AI_MOVE,
	IO_TIME,
	IO_GAME_EVENT
};

enum class Action 
{
	ACTION_NONE,
	HIDE_CURSOR, FIX_CURSOR, HIDE_AND_FIX_CURSOR,
	CHANGE_WIREFRAME_FLAG, CHANGE_BOUNDINGBOX_FLAG,
	PICKING,
	CUSTOM_FUNCTION_ONE, CUSTOM_FUNCTION_TWO, CUSTOM_FUNCTION_THREE,
	MOVE_FORWARD, MOVE_BACK, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN,
	TELEPORT_FORWARD, TELEPORT_BACK, TELEPORT_LEFT, TELEPORT_RIGHT, TELEPORT_UP, TELEPORT_DOWN,
	ROTATE, ROTATE_ROLL, ROTATE_PITCH, ROTATE_YAW, ROTATE_RIGHT, ROTATE_LOOK,
	ACTION_JUMP, ACTION_ONE, ACTION_TWO, ACTION_THREE, ACTION_FOUR, ACTION_FIVE,
	CHANGE_MOUSE, CHANGE_CAT
};

// 애니메이션 상태
// *33개가 넘어가면 Update시 State에 파싱 자료형 변경필요
enum class Object_State {
	STATE_NONE,
	STATE_IDLE, STATE_MOVE,
	STATE_JUMP_START, STATE_JUMP_IDLE, STATE_JUMP_END,
	STATE_ACTION_ONE, STATE_ACTION_TWO, STATE_ACTION_THREE, STATE_ACTION_FOUR, STATE_ACTION_FIVE,
	STATE_DEAD, STATE_STUN
};

enum class SOCKET_TYPE
{
	UDP_SOCKET,
	TCP_SOCKET
};

enum CHARACTER_NUMBER
{
	NUM_MOUSE1 = 0,
	NUM_MOUSE2,
	NUM_MOUSE3,
	NUM_MOUSE4,
	NUM_AI1,
	NUM_AI2,
	NUM_AI3,
	NUM_AI4,
	NUM_CAT,
	NUM_GHOST
};

enum class PLAYER_STATE
{
	PS_FREE,
	PS_ALLOC,
	PS_INGAME
};

enum class GAME_EVENT
{
	GE_OPEN_DOOR,
	GE_TIME_OVER,
	GE_WIN_CAT,
	GE_WIN_MOUSE,
	GE_ESCAPE,
	GE_REBORN,
	GE_DEAD
};

struct TIMER_EVENT {
	std::chrono::system_clock::time_point wakeup_time;
	int session_id;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeup_time > L.wakeup_time);
	}
};


struct Packet {
	int sequenceNumber;
	char data[BUF_SIZE];
};


struct Tile {
	int x, z;
	bool walkable;
};


extern SOCKET g_server_socket, g_client_socket;
extern HANDLE g_h_iocp;

extern concurrency::concurrent_priority_queue<TIMER_EVENT> timer_queue;
extern concurrency::concurrent_priority_queue<TIMER_EVENT> commandQueue;
extern concurrency::concurrent_priority_queue<TIMER_EVENT> AI_Queue;

constexpr float GRAVITY = 980.0f;
constexpr float FRICTION = 100.0f;
constexpr float STOP_THRESHOLD = 0.9f;	// 속도가 이 값보다 작아지면 멈추는 것으로 간주


extern void print_error(const char* msg, int err_no);
extern int GetSessionNumber(bool is_cat);
extern int GetWaitingPlayerNum();

//===========================================================
// Directx12
//===========================================================
#include <d3d12.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>


struct ObjectOBB
{
	std::string name;
	DirectX::BoundingOrientedBox obb;
	DirectX::XMVECTOR worldAxes[3];         // 충돌 계산을 위한 월드좌표계에서의 축
};
// 고양이 차징 점프 카메라 각 계산을 위한 값
constexpr float RIGHT_ANGLE_RADIAN = DirectX::XMConvertToRadians(70.0f);

#define EPSILON					1.0e-10f

inline bool IsZero(float fValue) { return((fabsf(fValue) < EPSILON)); }
inline bool IsEqual(float fA, float fB) { return(::IsZero(fA - fB)); }
inline float InverseSqrt(float fValue) { return 1.0f / sqrtf(fValue); }
inline void Swap(float* pfS, float* pfT) { float fTemp = *pfS; *pfS = *pfT; *pfT = fTemp; }

inline bool IsZeroVector(const DirectX::XMFLOAT3& vec)
{
	return vec.x == 0.0f && vec.y == 0.0f && vec.z == 0.0f;
}

// math
struct MathHelper {
	static float Infinity() { return FLT_MAX; }
	static float Pi() { return DirectX::XM_PI; }

	static float Rand_F() { return (float)rand() / (float)RAND_MAX; }
	static float Rand_F(float a, float b) { return a + Rand_F() * (b - a); }
	static int Rand_I() { return rand(); }
	static int Rand_I(int a, int b) { return a + rand() % ((b - a) + 1); }

	template<typename T>
	static T Min(const T& a, const T& b) { return a < b ? a : b; }

	template<typename T>
	static T Max(const T& a, const T& b) { return a > b ? a : b; }

	template<typename T>
	static T Lerp(const T& a, const T& b, float t) { return a + (b - a) * t; }

	template<typename T>
	static T Clamp(const T& x, const T& min, float max) { return x < min ? min : (x > max ? max : x); }

	static DirectX::XMMATRIX Inverse_Transpose(DirectX::XMMATRIX m) {
		m.r[3] = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

		DirectX::XMVECTOR determinant = DirectX::XMMatrixDeterminant(m);

		return DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(&determinant, m));
	}

	static DirectX::XMFLOAT4X4 Identity_4x4() {
		static DirectX::XMFLOAT4X4 identity{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		};

		return identity;
	}

	static DirectX::XMFLOAT2 Multiply(const DirectX::XMFLOAT2& xmfloat2, const DirectX::XMMATRIX& matrix) {
		DirectX::XMVECTOR vector = DirectX::XMLoadFloat2(&xmfloat2);

		DirectX::XMFLOAT2 result;
		DirectX::XMStoreFloat2(&result, DirectX::XMVector2Transform(vector, matrix));

		return result;
	}

	static DirectX::XMFLOAT3 Multiply(const DirectX::XMFLOAT3& xmfloat3, const DirectX::XMMATRIX& matrix) {
		DirectX::XMVECTOR vector = DirectX::XMLoadFloat3(&xmfloat3);

		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Transform(vector, matrix));

		return result;
	}

	static DirectX::XMFLOAT4X4 Multiply(const DirectX::XMFLOAT4X4& xmfloat4x4, const DirectX::XMMATRIX& matrix) {
		DirectX::XMFLOAT4X4 result;
		DirectX::XMStoreFloat4x4(&result, DirectX::XMMatrixMultiply(DirectX::XMLoadFloat4x4(&xmfloat4x4), matrix));

		return result;
	}

	//
	static DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Add(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b, float scalar) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorAdd(DirectX::XMLoadFloat3(&xmfloat3_a),
			DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmfloat3_b), scalar)));

		return result;
	}

	static DirectX::XMFLOAT3 Subtract(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Multiply(const DirectX::XMFLOAT3& xmfloat3, float scalar) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVectorScale(DirectX::XMLoadFloat3(&xmfloat3), scalar));

		return result;
	}

	static DirectX::XMFLOAT3 Normalize(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Normalize(DirectX::XMLoadFloat3(&xmfloat3)));

		return result;
	}

	static float Length(const DirectX::XMFLOAT3& xmfloat3) {
		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&xmfloat3)));
	}

	static float Length_XZ(const DirectX::XMFLOAT3& xmfloat3) {
		DirectX::XMFLOAT3 xmfloat3_xz(xmfloat3.x, 0.0f, xmfloat3.z);

		return DirectX::XMVectorGetX(DirectX::XMVector3Length(DirectX::XMLoadFloat3(&xmfloat3_xz)));
	}

	static DirectX::XMFLOAT3 Dot(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Dot(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}

	static DirectX::XMFLOAT3 Cross(const DirectX::XMFLOAT3& xmfloat3_a, const DirectX::XMFLOAT3 xmfloat3_b) {
		DirectX::XMFLOAT3 result;
		DirectX::XMStoreFloat3(&result, DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&xmfloat3_a), DirectX::XMLoadFloat3(&xmfloat3_b)));

		return result;
	}
};

// Math Helper XMFLOAT3 사용을 위한 오버로딩
// Overload + for XMFLOAT3
static DirectX::XMFLOAT3 operator+(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return MathHelper::Add(a, b);
}

// Overload - for XMFLOAT3
static DirectX::XMFLOAT3 operator-(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
	return MathHelper::Subtract(a, b);
}

// Overload * for scalar
static DirectX::XMFLOAT3 operator*(const DirectX::XMFLOAT3& a, float scalar) {
	return MathHelper::Multiply(a, scalar);
}

// Overload * for scalar
static DirectX::XMFLOAT3 operator*(float scalar, const DirectX::XMFLOAT3& a) {
	return MathHelper::Multiply(a, scalar);
}



//==========================
// Cheese Data
//==========================
constexpr int CHEESE_NUM = 4;
constexpr float CHEESE_SCALE = 1.0f;

extern const DirectX::XMFLOAT3 CHEESE_POS[CHEESE_NUM];

constexpr int VOXEL_CHEESE_HEIGHT = 8;
constexpr int VOXEL_CHEESE_DEPTH = 21;
constexpr int VOXEL_CHEESE_WIDTH = VOXEL_CHEESE_DEPTH / 2;
extern const int CHEESE_VOXEL_COUNT;

//==========================
// Character Initialize Data
//==========================
constexpr DirectX::XMFLOAT3 CHARACTER_POS[9] =
{
	DirectX::XMFLOAT3(10.0f, FLOOR_Y, 10.0f),					// Mouse1
	DirectX::XMFLOAT3(-10.0f, FLOOR_Y, 10.0f),					// Mouse2
	DirectX::XMFLOAT3(10.0f, FLOOR_Y, -10.0f),					// Mouse3
	DirectX::XMFLOAT3(-10.0f, FLOOR_Y, -10.0f),					// Mouse4
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI1
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI2
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI3
	DirectX::XMFLOAT3(0.0f, FLOOR_Y, 0.0f),						// AI4
	DirectX::XMFLOAT3(437.225f, FLOOR_Y, -117.493f)				// CAT
};

constexpr DirectX::XMFLOAT4 CHARACTER_ROTATION[9] =
{
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// Mouse1
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// Mouse2
	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),					// Mouse3
	DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f),					// Mouse4
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI1
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI2
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI3
	DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),					// AI4
	DirectX::XMFLOAT4(0.0f, -0.7071f, 0.0f, 0.7071f)			// CAT
};

