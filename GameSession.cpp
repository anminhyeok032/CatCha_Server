#include "GameSession.h"
#include "Character.h"

bool GameSession::Update()
{
	std::lock_guard<std::mutex> lock(mt_session_state_);

    bool need_update = false;
    uint64_t current_time = GetServerTime();
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;
    if (deltaTime < 50)
    {
        return false;
    }

    for (auto& character : characters_)
    {
        if (true == character.second->UpdatePosition(deltaTime)) need_update = true;
    }

    // ���� ���� �ð� ������Ʈ
    lastupdatetime_ = current_time;

    // Ŭ���̾�Ʈ���� ��ε�ĳ����
    if (true == need_update)
    {
        SendPlayerUpdate();
    }

    // TODO : �ð� ������� �и��Ұ�
    // 1�ʿ� �ѹ��� ���� ���� �ð� ��ε�ĳ����
    if (lastupdatetime_ - last_game_time_ >= 1000)
    {
        last_game_time_ = current_time;
        remaining_time_--;
        SendTimeUpdate();
    }
}

void GameSession::SendPlayerUpdate()
{
    // TODO : �ð��� 1�ʰ� ������ �����ְ� 
    //        ����� �÷��̾��� �����ӿ� ���� Send�� PQCS�� Worker �����忡�� ó��
}

void GameSession::SendTimeUpdate()
{
    SC_TIME_PACKET p;
    p.size = sizeof(SC_TIME_PACKET);
    p.type = SC_TIME;
    p.time = remaining_time_;

    for (auto& players : characters_)
    {
        players.second->DoSend(&p);
    }
}

uint64_t GameSession::GetServerTime()
{
    // ���� �ð� ��������
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}