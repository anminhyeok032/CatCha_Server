#include "GameSession.h"
#include "Character.h"

bool GameSession::Update()
{
	std::lock_guard<std::mutex> lock(mt_session_state_);

    bool need_update = false;
    uint64_t current_time = GetServerTime();
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // ���� updateTime�� 50ms==20fps���� ������ false
    if (deltaTime < 0.05f)
    {
        return false;
    }

    // ������ �÷��̾��� Postion�� ������Ʈ �ϵ��� int 8������ ������ ���θ� �Ľ��ؼ� ����
    int move_players = 0;
    for (auto& character : characters_)
    {
        if (true == character.second->UpdatePosition(deltaTime))
        {
            need_update = true;
            move_players |= (1 << character.second->comp_key_.player_index);
        }
    }

    // ���� ���� �ð� ������Ʈ
    lastupdatetime_ = current_time;

    // Ŭ���̾�Ʈ���� ��ε�ĳ����
    if (true == need_update)
    {
        SendPlayerUpdate(move_players);
    }
    commandQueue.push(session_num_);
    return true;
}

void GameSession::SendPlayerUpdate(int move_players)
{
    Over_IO* over = new Over_IO;
    over->io_key_ = IO_MOVE;
    CompletionKey* completion_key = new CompletionKey{ session_num_, move_players };
    PostQueuedCompletionStatus(g_h_iocp, 1, reinterpret_cast<ULONG_PTR>(completion_key), &over->over_);
}

void GameSession::SendTimeUpdate()
{
    SC_TIME_PACKET p;
    p.size = sizeof(p);
    p.type = SC_TIME;
    p.time = remaining_time_;

    for (auto& players : characters_)
    {
        players.second->DoSend(&p);
    }

    TIMER_EVENT ev{ std::chrono::system_clock::now(), session_num_ };
    timer_queue.push(ev);

}

void GameSession::BroadcastPosition(int player)
{
    SC_MOVE_PLAYER_PACKET p;
    p.size = sizeof(p);
    p.type = SC_MOVE_PLAYER;
    p.id = player;
    p.x = characters_[player]->x_;
    p.y = characters_[player]->y_;
    p.z = characters_[player]->z_;
    p.yaw = 0;
    std::cout << "Player Position : " << p.x << ", " << p.y << ", " << p.z << std::endl;
    for (auto& player : characters_)
    {
        player.second->DoSend(&p);
    }
}

uint64_t GameSession::GetServerTime()
{
    // ���� �ð� ��������
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}