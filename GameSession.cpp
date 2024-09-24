#include "GameSession.h"
#include "Character.h"


bool GameSession::Update()
{
	std::lock_guard<std::mutex> lock(mt_session_state_);

    bool need_update = false;
    uint64_t current_time = GetServerTime();
    float deltaTime = (current_time - lastupdatetime_) / 1000.0f;

    // 이전 updateTime이 50ms==20fps보다 적으면 false
    if (deltaTime < 0.05f)
    {
        return false;
    }

    // 움직인 플레이어의 Postion만 업데이트 하도록 int 8마리의 움직임 여부를 파싱해서 담음
    int move_players = 0;
    for (auto& character : characters_)
    {
        if (true == character.second->UpdatePosition(deltaTime))
        {
            need_update = true;
            move_players |= (1 << character.second->comp_key_.player_index);
        }
    }

    // 현재 세션 시간 업데이트
    lastupdatetime_ = current_time;

    // 클라이언트에게 브로드캐스팅
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

void GameSession::InitUDPSocket()
{
    // UDP 소켓 설정
    udp_socket_ = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (udp_socket_ == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    // Bind socket to port
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDPPORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_socket_, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return;
    }

    CompletionKey completion_key{ session_num_, -1 };
    CreateIoCompletionPort(reinterpret_cast<HANDLE>(udp_socket_), g_h_iocp, reinterpret_cast<ULONG_PTR>(&completion_key), 0);
    
    Over_IO* over = new Over_IO();
    DWORD flag = 0;
    int res = WSARecvFrom(udp_socket_, &over->wsabuf_, 1, nullptr, &flag, reinterpret_cast<sockaddr*>(&over->clientAddr_),
        &over->clientAddrLen_, &over->over_, nullptr);
    if (res == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
    {
        print_error("WSARecvFrom", WSAGetLastError());
    }
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
    /*for (auto& player : characters_)
    {
        player.second->DoSend(&p);
    }*/
    
    for (auto& player : characters_) // 세션에 속한 클라이언트 목록
    {
        int res = sendto(udp_socket_, (char*)&p, sizeof(p), 0,
            (sockaddr*)&player.second->client_addr_.sin_addr, sizeof(player.second->client_addr_));
        if (res != 0)
        {
            print_error("udp", WSAGetLastError());
        }
    }
}

uint64_t GameSession::GetServerTime()
{
    // 서버 시간 가져오기
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count();
}