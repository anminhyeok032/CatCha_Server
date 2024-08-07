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

    // 현재 세션 시간 업데이트
    lastupdatetime_ = current_time;

    // 클라이언트에게 브로드캐스팅
    if (true == need_update)
    {
        SendPlayerUpdate();
    }

    // TODO : 시간 스레드는 분리할것
    // 1초에 한번씩 게임 남은 시간 브로드캐스팅
    if (lastupdatetime_ - last_game_time_ >= 1000)
    {
        last_game_time_ = current_time;
        remaining_time_--;
        SendTimeUpdate();
    }
}

void GameSession::SendPlayerUpdate()
{
    // TODO : 시간은 1초가 지나면 보내주고 
    //        변경된 플레이어의 움직임에 대한 Send는 PQCS로 Worker 스레드에서 처리
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
    // 서버 시간 가져오기
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}