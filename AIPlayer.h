#pragma once
#include "Character.h"

struct Node {
	int x, z;
	float gCost, hCost;
	Node* parent;

	float GetFCost() const { return gCost + hCost; }
	bool operator>(const Node& other) const { return GetFCost() > other.GetFCost(); }
};

class AIPlayer : public Character
{
public:
	int current_x_;
	int current_z_;

	bool is_reached_ = false;

	// A*로 찾은 경로
	std::vector<std::pair<int, int>> path_;

	DirectX::BoundingSphere bounding_sphere_{ DirectX::XMFLOAT3{0, FLOOR_Y, 0}, 10.92712f / 2.0f };

public:

	AIPlayer()
	{
		character_id_ = NUM_GHOST;
		key_ = 0;
		socket_ = INVALID_SOCKET;
		current_x_ = TILE_MAP_WIDTH / TILE_SIZE / 2;
		current_z_ = TILE_MAP_LENGTH / TILE_SIZE / 2;
	}

	~AIPlayer()
	{
		std::cout << "AIPlayer Destructor" << std::endl;
	}

	// Player 초기화를 위한 가상 함수
	virtual void SetSocket(SOCKET socket) {}
	virtual void DoReceive() {}
	virtual void ProcessPacket(char* packet) {}
	virtual void DoSend(void* packet) {}
	virtual void SetCompletionKey(CompletionKey& key) {}

	SOCKET GetSocket() override { return SOCKET_ERROR; }
	CompletionKey GetCompletionKey() override { return {nullptr, nullptr}; }

	void SetID(int id) override { character_id_ = id; }
	void SetBoundingSphere();
	// 휴리스틱 함수 (맨해튼 거리 사용해서 대각선 이동 불가능하게)
	int ManhattanDistance(int x1, int y1, int x2, int y2);
	// AIPlayer A* 경로 설정
	void FindPath(int target_x, int target_z);

	bool UpdatePosition(float deltaTime);

};